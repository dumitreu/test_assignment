#pragma once

#include <commondefs.hpp>
#include <json.hpp>
#include <file_util.hpp>
#include <str_util.hpp>
#include <threading.hpp>

#include "misc_utils.hpp"
#include "configuration.hpp"
#include "globals.hpp"
#include "logging.hpp"

// a schedule of unsuccessfull fetching attempts to try again later
class fetch_schedule final {
public:
    fetch_schedule(std::string const &file_name):
        file_name_{file_name}
    {
        load();
    }

    bool schedule(std::string const &date) {
        if(is_valid_yyyymmdd_date(date)) {
            schd_[date] = true;
            return save();
        }
        return false;
    }

    bool unschedule(std::string const &date) {
        if(scheduled(date)) {
            schd_.key_remove(date);
            return save();
        }
        return false;
    }

    bool scheduled(std::string const &date) {
        return schd_.is_object() && schd_.key_exists(date);
    }

    std::vector<std::string> all_scheduled_dates() {
        std::vector<std::string> res{};
        if(schd_.is_object()) {
            res.reserve(schd_.size());
            schd_.traverse_object([&](std::string const &key, lins::json::object const &) { res.push_back(key); });
        }
        return res;
    }

private:
    bool load() {
        if(lins::file_util::file_exists(file_name_)) {
            std::vector<std::uint8_t> file_content{};
            try {
                file_content = lins::file_util::load_from_file(file_name_);
                schd_ = lins::json::object::deserialize({file_content.begin(), file_content.end()});
                return true;
            } catch (std::exception const &e) {
                schd_.become_null();
                LOG << "file loading error: " << e.what() << std::endl;
            }
        }
        return false;
    }

    bool save() {
        if(schd_.is_object()) {
            try {
                return lins::file_util::save_to_file(file_name_, schd_.serialize(false));
            } catch (std::exception const &e) {
                LOG << "file saving error: " << e.what() << std::endl;
            }
        }
        return false;
    }

private:
    std::string file_name_{};
    lins::json::object schd_{};
};
