#pragma once

#include <commondefs.hpp>
#include <json.hpp>
#include <file_util.hpp>
#include <str_util.hpp>
#include <threading.hpp>
#include <timespec_wrapper.hpp>

static bool is_valid_yyyymmdd_date(std::string const &date) {
    bool res{date.size() == 8};
    std::for_each(date.begin(), date.end(), [&](auto &&c) { res = res && std::isdigit(c); });
    return res;
}

static bool valid_dotted_date(std::string const &dot_date, lins::timespec_wrapper *found_value = nullptr) {
    bool res{false};
    try {
        auto str_list{lins::str_util::str_tok<std::string>(dot_date, ".")};
        if(str_list.size() == 3) {
            int d{(int)lins::str_util::atoi(str_list[0])};
            int m{(int)lins::str_util::atoi(str_list[1])};
            int y{(int)lins::str_util::atoi(str_list[2])};

            std::stringstream ss{};
            ss << std::setfill('0') << std::setw(4) << y << "-" << std::setw(2) << m << "-" << std::setw(2) << d;
            std::string ctl_date{ss.str()};
            lins::timespec_wrapper chck{ctl_date};
            if(chck.day() == d && chck.month() == m && chck.year() == y) {
                if(found_value) {
                    *found_value = chck;
                }
                res = true;
            }
        }
    } catch (...) {
        res = false;
    }
    return res;
}

static std::string dotted_date_to_yyyymmdd(std::string const &dot_date) {
    std::string res{};
    try {
        lins::timespec_wrapper contained_val{};
        if(valid_dotted_date(dot_date, &contained_val)) {
            std::stringstream ss{};
            ss << std::setfill('0') << std::setw(4) << contained_val.year() << std::setw(2) << contained_val.month() << std::setw(2) << contained_val.day();
            res = ss.str();
        }
    } catch (...) {
        res.clear();
    }
    return res;
}

static bool is_valid_currencies_array(lins::json::object const &currencies) {
    bool res{true};
    if(currencies.is_array() && currencies.size() > 0) {
        for(std::size_t i{0}; i < currencies.size(); ++i) {
            lins::json::object const &curr_entry{currencies[i]};
            if(
                !curr_entry.is_object() ||
                !curr_entry.string_field_exists("cc") ||
                !curr_entry.string_field_exists("exchangedate") || !valid_dotted_date(curr_entry["exchangedate"].as_string()) ||
                !curr_entry.key_exists("r030") || !curr_entry["r030"].is_number() ||
                !curr_entry.key_exists("rate") || !(curr_entry["rate"].is_real() || curr_entry["rate"].is_number()) ||
                !curr_entry.string_field_exists("txt")
            ) {
                res = false;
                break;
            }
        }
    } else {
        res = false;
    }
    return res;
}

static int current_day() {
    return lins::timespec_wrapper::now().day();
}

static int current_month() {
    return lins::timespec_wrapper::now().month();
}

static int current_year() {
    return lins::timespec_wrapper::now().year();
}

static std::string date_right_now() {
    std::stringstream res{};
    lins::timespec_wrapper tsw{lins::timespec_wrapper::now()};
    res << std::setfill('0') << std::setw(4) << tsw.year() << std::setw(2) << tsw.month() << std::setw(2) << tsw.day();
    return res.str();
}

static bool good_file_name_or_candidate(std::string const &file_name) {
    return lins::file_util::file_exists(file_name) ||
           lins::file_util::dir_exists(lins::file_util::extract_file_dir(file_name)) ||
           (lins::file_util::extract_file_dir(file_name).empty() && !file_name.empty());
}
