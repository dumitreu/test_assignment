#include "commondefs.hpp"
#include "json.hpp"
#ifdef USE_SIMDJSON
#include "simdjson/include/simdjson.h"
#include "simdjson/src/simdjson.cpp"
#endif

namespace lins::json {

#ifdef USE_SIMDJSON
    namespace detail {

        object translate(simdjson::dom::element const &from) {
            object res{};
            if(from.is_null()) {
                res.become_null();
            } else if(from.is_string()) {
                res = std::string{from.get_string().value()};
            } else if(from.is_bool()) {
                res = from.get_bool().value();
            } else if(from.is_int64() || from.is_uint64()) {
                res = from.get_int64().value();
            } else if(from.is_double()) {
                res = (long double)from.get_double().value();
            } else if(from.is_array()) {
                res.become_array();
                simdjson::dom::array arr{from.get_array()};
                for(auto it{arr.begin()}; it != arr.end(); ++it) {
                    res.push_back(translate(*it));
                }
            } else if(from.is_object()) {
                res.become_object();
                simdjson::dom::object obj{from.get_object()};
                for(auto it{obj.begin()}; it != obj.end(); ++it) {
                    res[std::string{it.key_c_str()}] = translate(it.value());
                }
            }
            return res;
        }

    }
#endif

    object object::deserialize(std::string_view const &s, bool use_hw_acclr) {
#ifdef USE_SIMDJSON
        if(use_hw_acclr) {
            simdjson::dom::parser parser{};
            simdjson::dom::element sj = parser.parse(s.data(), s.size());
            return detail::translate(sj);
        } else {
#endif
            static const std::string parse_string{
                "#rd|'\\\"'>>str;"
                "str|$df>>str|'\\\"'>><str|'\\\\'>>stre;"
                "stre|$df>>str;"
                "#rd|'\\''>>sts;"
                "sts|$df>>sts|'\\''>><sts|'\\\\'>>stse;"
                "stse|$df>>sts;"
                "#rd|'/'>>/;"
                "/|$df>>^/|'*'>>mlc|'/'>>slc;"
                "mlc|$df>>mlc|'*'>>mlcs;"
                "#rd|'#'>>slc;"
                "slc|$df>>slc|[\r\n]>>^//;"
                "mlcs|$df>>mlc|'/'>><//;"
                "#rd|[:space:]>>spc;"
                "spc|[:space:]>>spc|$df>>^spc;"
                "#rd|[+-]>>sign;"
                "sign|[:digit:]>>int|'.'>>mbfp;"
                "#rd|[:digit:]>>int;"
                "#rd|'.'>>mbfp;"
                "mbfp|[:digit:]>>fp;"
                "int|'.'>>fpd|[:punct::space:]>>^int|[eE]>>fpe1|[:digit:]>>int;"
                "fpd|[:digit:]>>fp|[eE]>>fpe1;"
                "fp|[:punct::space:]>>^fpl|[:digit:]>>fp|[eE]>>fpe1;"
                "fpe1|[-+]>>fpes|[:digit:]>>fpe2;"
                "fpes|[:digit:]>>fpe2|'.'>>fpesd;"
                "fpesd|[:digit:]>>fpesd|[:punct::space:]>>^fpl;"
                "fpe2|[:digit:]>>fpe2|'.'>>fpe2d|[:punct::space:]>>^fpl;"
                "fpe2d|[:digit:]>>fpe2d|[:punct::space:]>>^fpl;"
                "#rd|'n'>>n;"
                "n|'u'>>nu|'i'>>ni;"
                "nu|'l'>>nul;"
                "ni|'l'>>nil;"
                "nul|'l'>>null;"
                "null|[:punct::space:]>>^nul;"
                "nil|[:punct::space:]>>^nul;"
                "#rd|'t'>>t;"
                "t|'r'>>tr;"
                "tr|'u'>>tru;"
                "tru|'e'>>true;"
                "true|[:punct::space:] >> ^tru;"
                "#rd|'f'>>f;"
                "f|'a'>>fa;"
                "fa|'l'>>fal;"
                "fal|'s'>>fals;"
                "fals|'e'>>false;"
                "false|[:punct::space:]>>^fal;"
                "#rd|','>><,;"
                "#rd|':'>><:;"
                "#rd|'['>><[;"
                "#rd|']'>><];"
                "#rd|'{'>><{;"
                "#rd|'}'>><};"
            };
            object_deserializer d{};
            lins::fsm_tokenizer<std::string> t{parse_string};
            t.set_listener_delegate([&](lins::any const &a) {
                d.any_value_observer_accept_value(a);
            });
            t.accept_string(std::string{s});
            t.finalize();
            return d.harvest_result();
#ifdef USE_SIMDJSON
        }
#endif
    }

    object object::deserialize(std::string const &s, bool use_hw_acclr) {
        return deserialize(std::string_view{s.data(), s.size()}, use_hw_acclr);
    }

    object object::deserialize(char const *s, bool use_hw_acclr) {
        return deserialize(std::string_view{s, std::strlen(s)}, use_hw_acclr);
    }

}
