#pragma once

#include <commondefs.hpp>
#include <json.hpp>
#include <file_util.hpp>
#include <str_util.hpp>
#include <terminable.hpp>
#include <threading.hpp>
#include <command_queue.hpp>
#include <http_client/httprequest.hpp>
#include <croncpp.h>

#include "misc_utils.hpp"
#include "api_queries_composer.hpp"
#include "configuration.hpp"
#include "fetch_schedule.hpp"
#include "rates_storage.hpp"
#include "globals.hpp"
#include "logging.hpp"

class exchange_rates_fetch_logic final: public lins::terminable {
public:
    exchange_rates_fetch_logic() = default;

    exchange_rates_fetch_logic(exchange_rates_fetch_logic const &) = delete;

    exchange_rates_fetch_logic(exchange_rates_fetch_logic &&) = delete;

    exchange_rates_fetch_logic &operator=(exchange_rates_fetch_logic const &) = delete;

    exchange_rates_fetch_logic &operator=(exchange_rates_fetch_logic &&) = delete;

    ~exchange_rates_fetch_logic() {
        terminate();
        if(cron_thread_.joinable()) { cron_thread_.join(); }
        if(fetch_scheduled_thread_.joinable()) { fetch_scheduled_thread_.join(); }
    }

    bool init() {
        if(!initialized_) {
            strg_--->load(config()--->data_file_name());

            cron_thread_ = std::thread{[this]() {
                LOG << "starting thread cron_thread_" << std::endl;
                std::string cronstr{config()--->cron_string()};
                auto cron = cron::make_cron(cronstr);
                for(std::time_t next{cron::cron_next(cron, std::time(0))}; !termination();) {
                    std::time_t now{std::time(0)};
                    if(cronstr != config()--->cron_string()) {
                        cronstr = config()--->cron_string();
                        cron = cron::make_cron(cronstr);
                        next = cron::cron_next(cron, now);
                    }
                    if(now > next) {
                        next = cron::cron_next(cron, now);
                        if(global_regular_queue) {
                            global_regular_queue->add_job([this]() {
                                fetch_today_rates();
                            });
                        } else {
                            LOG << "global regular queue is a null pointer" << std::endl;
                        }
                    }
                    // for the reason of cron we cannot wait on some synchro object...
                    std::this_thread::sleep_for(std::chrono::milliseconds{200});
                }
            }};

            // this thread performs fetching of the re-scheduled exchange rates downloads, due to failure
            // fetching attempt by normal schedule from config
            fetch_scheduled_thread_ = std::thread{[this]() {
                std::string cronstr{config()--->failed_sched_cron_string()};
                auto cron = cron::make_cron(cronstr);
                for(std::time_t next{cron::cron_next(cron, std::time(0))}; !termination();) {
                    std::time_t now{std::time(0)};
                    if(cronstr != config()--->failed_sched_cron_string()) {
                        cronstr = config()--->failed_sched_cron_string();
                        cron = cron::make_cron(cronstr);
                        next = cron::cron_next(cron, now);
                    }
                    if(now > next) {
                        LOG << "checking for scheduled failed tasks" << std::endl;
                        next = cron::cron_next(cron, now);
                        if(global_retry_queue) {
                            global_retry_queue->add_job([this]() {
                                for(std::string lsd{get_last_scheduled()}; !lsd.empty(); lsd = get_last_scheduled()) {
                                    if(is_valid_yyyymmdd_date(lsd)) {
                                        fetch_scheduled_for_date(lsd);
                                    } else {
                                        unschedule_fetching(lsd);
                                    }
                                }
                            });
                        } else {
                            LOG << "global retry queue is a null pointer" << std::endl;
                        }
                    }
                    // for the reason of cron we cannot wait on some synchro object...
                    std::this_thread::sleep_for(std::chrono::milliseconds{200});
                }
            }};

            initialized_ = true;

            // single-shot fetching action being performed upon (re)start
            if(global_regular_queue) {
                global_regular_queue->add_job([this]() { fetch_today_rates(); });
            } else {
                LOG << "global regular queue is a null pointer" << std::endl;
            }
        }
        return initialized_;
    }

    bool ok() const noexcept {
        return initialized_;
    }

private:
    void schedule_fetching(std::string const &for_date) {
        std::lock_guard l{sched_file_mtp_};
        fetch_schedule fs{config()--->sched_file_name()};
        fs.schedule(for_date);
    }

    void unschedule_fetching(std::string const &for_date) {
        std::lock_guard l{sched_file_mtp_};
        fetch_schedule fs{config()--->sched_file_name()};
        fs.unschedule(for_date);
    }

    std::string get_last_scheduled() const {
        std::lock_guard l{sched_file_mtp_};
        fetch_schedule fs{config()--->sched_file_name()};
        auto schs{fs.all_scheduled_dates()};
        if(!schs.empty()) {
            return schs[schs.size() - 1];
        }
        return {};
    }

    bool fetch_scheduled_for_date(std::string const &date) {
        // attempt to recover missing data gap scheduled after
        // fetching failure from the remote service
        if(fetch_rates(date)) {
            unschedule_fetching(date);
            return true;
        }
        return false;
    }

    void fetch_today_rates() {
        std::string cur_date{date_right_now()};
        if(!fetch_rates(cur_date)) {
            // error handling for the case where
            // the service cannot retrieve data
            schedule_fetching(cur_date);
        }
    }

    bool fetch_rates(std::string const &date) {
        bool processed{false};
        try {
            std::string url{api_queries_composer::given_date_rates(date, config()--->fetched_data_type())};
            LOG << "requested url: " << url << " for date " << date << std::endl;
            auto resp{http::Request{url}.send()};
            if(resp.status.code == 200) {
                LOG << "remote server response: " << resp.status.code << " searching content-type header..." << std::endl;
                for(auto &&hp: resp.headerFields) {
                    if(lins::str_util::strtolower(hp.first) == "content-type") {
                        LOG << "found " << hp.first << "!" << std::endl;
                        auto sc_tokens{lins::str_util::str_tok<std::string>(hp.second, ";")};
                        if(sc_tokens.size()) {
                            auto sl_tokens{lins::str_util::str_tok<std::string>(sc_tokens[0], "/")};
                            for(auto &&sl_tk: sl_tokens) {
                                if(lins::str_util::strtolower(sl_tk) == "json") {
                                    LOG << "content type: " << lins::str_util::strtolower(sl_tk) << " - processing!" << std::endl;
                                    lins::json::object curr_data{lins::json::object::deserialize({resp.body.begin(), resp.body.end()})};
                                    if(is_valid_currencies_array(curr_data)) {
                                        LOG << "body data is a valid JSON" << std::endl;
                                        strg_--->add(curr_data);
                                        LOG << "flushing db to file: " << config()--->data_file_name() << std::endl;
                                        strg_--->save(config()--->data_file_name());
                                        processed = true;
                                        break;
                                    }
                                } else {
                                    LOG << "content type: " << lins::str_util::strtolower(sl_tk) << " - skipping..." << std::endl;
                                }
                            }
                        }
                    } else {
                        LOG << "header " << hp.first << " - skipping" << std::endl;
                    }
                    if(processed) {
                        break;
                    }
                }
            }
        } catch(std::exception const &e) {
            LOG << "rates fetching error: " << e.what() << std::endl;
        }
        return processed;
    }

private:
    bool initialized_{false};
    lins::thread_safe_wrapper_stl<storage> strg_{};
    std::thread cron_thread_{};
    std::thread fetch_scheduled_thread_{};
    mutable std::mutex sched_file_mtp_{};
};
