#pragma once

#include <commondefs.hpp>
#include <json.hpp>
#include <file_util.hpp>
#include <str_util.hpp>
#include <threading.hpp>

#include "configuration.hpp"
#include "misc_utils.hpp"
#include "globals.hpp"
#include "logging.hpp"

class api_queries_composer final {
public:
    enum class format {
        JSON,
        XML,
    };

    static format str_format(std::string const &fmt_str) {
        if(lins::str_util::strtolower(fmt_str) == "xml") {
            return format::XML;
        }
        return format::JSON;
    }

    static std::string current_date_rates(std::string const &fmt_str = "json") {
        format fmt{str_format(fmt_str)};
        std::string res{"https://bank.gov.ua/NBUStatService/v1/statdirectory/exchange"};
        switch(fmt) {
            case format::JSON:
                return res + "?json";
            case format::XML:
                break;
            default:
                res.clear();
                break;
        }
        return res;
    }

    static std::string given_date_rates(std::string const &date, std::string const &fmt_str = "json") {
        format fmt{str_format(fmt_str)};
        if(is_valid_yyyymmdd_date(date)) {
            std::stringstream ss{};
            ss << "https://bank.gov.ua/NBUStatService/v1/statdirectory/exchange?date=" << date;
            switch(fmt) {
                case format::JSON:
                    ss << "&json";
                    break;
                case format::XML:
                    break;
                default:
                    return {};
            }
            return ss.str();
        }
        return {};
    }
};
