#pragma once

#include <commondefs.hpp>
#include <json.hpp>
#include <file_util.hpp>
#include <str_util.hpp>

#include <croncpp.h>

#include "misc_utils.hpp"
#include "logging.hpp"

#ifdef PLATFORM_WINDOWS
#include <debugapi.h>
#endif

// json file configuration
class configuration {
public:
    class configuration_storage_error final: public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

public:
    bool load(
        std::string const &filepath,
        bool create_when_not_exists = true
    ) {
        bool result{false};
        filepath_ = filepath;
        if(lins::file_util::file_exists(filepath_)) {
            result = reload();
        } else if(create_when_not_exists) {
            result = store();
        }
        return result;
    }

    bool store() {
        bool result{false};
        try {
            lins::json::object cfg{};
            cfg["cron_string"] = cron_string_;
            cfg["failed_sched_cron_string"] = failed_sched_cron_string_;
            cfg["fetched_data_type"] = fetched_data_type_;
            cfg["data_file_name"] = data_file_name_;
            cfg["sched_file_name"] = sched_file_name_;
            cfg["enable_log_file"] = enable_log_file_;
            cfg["log_file_name"] = log_file_name_;
            result = lins::file_util::save_to_file(filepath_, cfg.serialize(false));
        } catch (std::exception const &e) {
#ifdef PLATFORM_WINDOWS
            lins::str_util::tstring outstr{ _T("saving config error : ") };
#ifdef UNICODE
            outstr += lins::str_util::from_utf8(e.what());
#else
            outstr += e.what();
#endif
            OutputDebugString(outstr.c_str());
#endif
        }
        return result;
    }

    bool reload() {
        bool result{false};
        if(lins::file_util::file_exists(filepath_)) {
            try {
                // load config file
                auto file_content{lins::file_util::load_from_file(filepath_)};
                lins::json::object cfg{lins::json::object::deserialize({file_content.begin(), file_content.end()})};

                // check config file values
                std::string cron_string{cfg["cron_string"].as_string()};
                if(cron::INVALID_TIME == cron::cron_next(cron::make_cron(cron_string), std::time(0))) {
                    throw configuration_storage_error{"configuration: cron string error"};
                }
                std::string failed_sched_cron_string{cfg["failed_sched_cron_string"].as_string()};
                if(cron::INVALID_TIME == cron::cron_next(cron::make_cron(failed_sched_cron_string), std::time(0))) {
                    throw configuration_storage_error{"configuration: failed sched cron string error"};
                }
                std::string fetched_data_type{lins::str_util::strtolower(cfg["fetched_data_type"].as_string())};
                if(!(fetched_data_type == "json" || fetched_data_type == "xml")) {
                    throw configuration_storage_error{"configuration: fetched data type error"};
                }
                if(fetched_data_type == "xml") {
                    throw configuration_storage_error{"configuration: xml format is not supported for now..."};
                }
                std::string data_file_name{cfg["data_file_name"].as_string()};
                if(!good_file_name_or_candidate(data_file_name)) {
                    throw configuration_storage_error{"configuration: data file name error"};
                }
                std::string sched_file_name{cfg["sched_file_name"].as_string()};
                if(!good_file_name_or_candidate(sched_file_name)) {
                    throw configuration_storage_error{"configuration: sched file name error"};
                }

                bool enable_log_file{false};
                if(cfg.key_exists("enable_log_file")) {
                    enable_log_file = cfg["enable_log_file"].as_boolean();
                }
                std::string log_file_name{cfg["log_file_name"].as_string()};
                if(enable_log_file && !good_file_name_or_candidate(log_file_name)) {
                    throw configuration_storage_error{"configuration: sched file name error"};
                }

                // assign values
                cron_string_ = cron_string;
                failed_sched_cron_string_ = failed_sched_cron_string;
                fetched_data_type_ = fetched_data_type;
                data_file_name_ = data_file_name;
                sched_file_name_ = sched_file_name;
                enable_log_file_ = enable_log_file;
                log_file_name_ = log_file_name;

                result = true;
            } catch (std::exception const &e) {
#ifdef PLATFORM_WINDOWS
                lins::str_util::tstring outstr{ _T("reloading config error : ") };
#ifdef UNICODE
                outstr += lins::str_util::from_utf8(e.what());
#else
                outstr += e.what();
#endif
                OutputDebugString(outstr.c_str());
#endif
            }
        }
        return result;
    }

    void set_defaults() noexcept {
        cron_string_ = "0 0 6 * * *";
        failed_sched_cron_string_ = "0 */5 * * * *";
        fetched_data_type_ = "json";
        data_file_name_ = "CurrRatesService.db";
        sched_file_name_ = "CurrRatesService.bad";
        enable_log_file_ = true;
        log_file_name_ = "CurrRatesService.log";
    }

    INLINE_GETTER_SETTER(std::string, cron_string, cron_string_)
    INLINE_GETTER_SETTER(std::string, failed_sched_cron_string, failed_sched_cron_string_)
    INLINE_GETTER_SETTER(std::string, fetched_data_type, fetched_data_type_)
    INLINE_GETTER_SETTER(std::string, data_file_name, data_file_name_)
    INLINE_GETTER_SETTER(std::string, sched_file_name, sched_file_name_)
    INLINE_GETTER_SETTER(bool, enable_log_file, enable_log_file_)
    INLINE_GETTER_SETTER(std::string, log_file_name, log_file_name_)

private:
    std::string filepath_{};
    
    // data fields with default values
    std::string cron_string_{"0 0 6 * * *"}; // perform every morning, 6:00
    std::string failed_sched_cron_string_{"0 */5 * * * *"}; // perform every 5 minutes
    std::string fetched_data_type_{"json"}; // default format
    std::string data_file_name_{"currency_rates.json"}; // "database" name
    std::string sched_file_name_{"rates_fetch_sched.json"}; // unsuccessfull fetching attempts retry schedule
    bool enable_log_file_{false}; // log to file when logging is enabled
    std::string log_file_name_{"currency_rates_svc.log"}; // name for currency rates downloader log file
};
