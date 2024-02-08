#pragma once

#include <commondefs.hpp>
#include <json.hpp>
#include <threading.hpp>
#include <file_util.hpp>
#include <str_util.hpp>

#include "misc_utils.hpp"
#include "globals.hpp"
#include "logging.hpp"

class storage final {
public:
    bool load(std::string const &file_name) noexcept {
        bool result{false};
        try {
            data_--->clear();
            LOG << "loading storage from file " << file_name << std::endl;
            auto file_content{lins::file_util::load_from_file(file_name)};
            LOG << "parsing json data..." << std::endl;
            data_ = lins::json::object::deserialize({file_content.begin(), file_content.end()});
            result = true;
        } catch (std::exception const &e) {
            LOG << "storage loading error " << e.what() << std::endl;
        }
        return result;
    }

    void save(std::string const &file_name) noexcept {
        try {
            LOG << "storage saving data to file " << file_name << std::endl;
            lins::file_util::save_to_file(file_name, data_--->serialize(false));
        } catch (std::exception const &e) {
            LOG << "storage saving error " << e.what() << std::endl;
        }
    }

    long double get_exchange_rate(int day, int month, int year, std::string const &currency_name) const {
        long double res{0};
        try {
            std::stringstream ss{};
            ss << std::setfill('0') << std::setw(4) << year << std::setw(2) << month << day;
            std::string key_date{ss.str()};
            if((**data_)["rates"].key_exists(key_date)) {
                // nesting due to locking
                if((**data_)["rates"][key_date].key_exists(currency_name)) {
                    res = (**data_)["rates"][key_date][currency_name].as_longdouble(0);
                }
            }
        } catch (...) {
            res = 0;
        }
        return res;
    }

    long double get_actual_ex_rate(std::string const &currency_name) const {
        long double res{0};
        LOG << "searching actual rate for " << currency_name << std::endl;
        try {
            if(data_--->key_exists("rates")) {
                // nesting due to locking
                if((**data_)["rates"].size()) {
                    auto indx{(**data_)["rates"].size() - 1};
                    lins::json::object const &max_date{(**data_)["rates"][indx]};
                    if(max_date.key_exists(currency_name)) {
                        res = max_date[currency_name].as_longdouble(0);
                        LOG << "found " << res << " for " << currency_name << std::endl;
                    } else {
                        LOG << currency_name << " not found for the latest date" << std::endl;
                    }
                } else {
                    LOG << "the related table of the DB is empty" << std::endl;
                }
            } else {
                LOG << "the DB is empty" << std::endl;
            }
        } catch (...) {
            res = 0;
        }
        return res;
    }

    void add(lins::json::object const &exchange_rates) {
        try {
            if(exchange_rates.is_array()) {
                LOG << "adding data to the DB: " << exchange_rates.serialize(false) << std::endl;
                for(std::size_t i = 0; i < exchange_rates.size(); ++i) {
                    lins::json::object const &rec{exchange_rates[i]};
                    check_curr_dict(rec);
                    std::string db_date{dotted_date_to_yyyymmdd(rec["exchangedate"].as_string())};
                    if(db_date.size() == 8) {
                        (**data_)["rates"][db_date][rec["cc"].as_string()] = rec["rate"].try_as_long_double();
                    }
                }
            }
        } catch (...) {
        }
    }

private:
    void check_curr_dict(lins::json::object const &rec) {
        try {
            if(!(**data_)["currencies"].key_exists(rec["cc"].as_string())) {
                LOG << "adding data currency record to dictionary: " << rec.serialize(false) << std::endl;
                (**data_)["currencies"][rec["cc"].as_string()]["r030"] = rec["r030"].as_number();
                (**data_)["currencies"][rec["cc"].as_string()]["txt"] = rec["txt"].as_string();
            }
        } catch (...) {
        }
    }

    lins::thread_safe_wrapper_stl<lins::json::object> data_{};
};
