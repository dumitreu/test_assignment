#pragma once

#include "commondefs.hpp"

#include "any.hpp"
#include "fsm_tokenizer.hpp"
#include "set_of.hpp"
#include "str_util.hpp"
#include "timespec_wrapper.hpp"
#include "base64.hpp"

namespace lins::json {

    DEFINE_RUNTIME_EXCEPTION_CLASS(json_error)

    namespace convert {

        inline std::string itoa(int64_t v) {
            return str_util::itoa(v);
        }

        inline std::string uitoa(uint64_t v) {
            return str_util::utoa(v);
        }

        template<typename T>
        inline std::string ftoa(T v, std::size_t prec = 6) {
            std::ostringstream s;
            s << std::setprecision(prec) << std::fixed << v;
            std::string res{s.str()};
            auto pos{res.find('.')};
            if(pos == std::string::npos) { pos = res.find(','); }
            if(pos != std::string::npos) {
                auto i{res.size() - 1};
                while(i > pos) {
                    if(res[i] == '0') {
                        --i;
                    } else {
                        break;
                    }
                }
                res = res.substr(0, i + 1);
                if(res.size() && (res[res.size() - 1] == '.' || res[res.size() - 1] == ',')) {
                    res = res.substr(0, res.size() - 1);
                }
            }
            return res;
        }

        inline int64_t atoi(std::string const &v) {
            return str_util::atoi(v);
        }

        inline long double atof(std::string const &v) {
            return std::atof(v.c_str());
        }

        static std::wstring wescape(std::wstring const &ws) {
            std::wstringstream res{};
            for(std::wstring::size_type i = 0; i < ws.size(); i++) {
                wchar_t c = ws[i];
                switch (c) {
                    case '\"': case '\\': res << '\\' << c; break;
                    case '\b': res << '\\' << 'b'; break;
                    case '\f': res << '\\' << 'f'; break;
                    case '\n': res << '\\' << 'n'; break;
                    case '\r': res << '\\' << 'r'; break;
                    case '\t': res << '\\' << 't'; break;
                    default: res << c; break;
                }
            }
            return res.str();
        }

        static std::string escape(std::string const &s) {
            return str_util::to_utf8(wescape(str_util::from_utf8(s)));
        }

        static std::wstring wunescape(std::wstring const &ws) {
            std::wstringstream res{};
            bool escp{false};
            for(std::size_t i = 0; i < ws.size(); ++i) {
                wchar_t c = ws[i];
                if(escp) {
                    switch (c) {
                        case 'b': res << '\b'; break;
                        case 'n': res << '\n'; break;
                        case 'r': res << '\r'; break;
                        case 't': res << '\t'; break;
                        case 'f': res << '\f'; break;
                        default: res << c; break;
                    }
                    escp = false;
                } else {
                    if(c == '\\') {
                        escp = true;
                    } else {
                        res << c;
                    }
                }
            }
            return res.str();
        }

        static std::string unescape(std::string const &s) {
            return str_util::to_utf8(wunescape(str_util::from_utf8(s)));
        }

        static std::wstring wunescape_s(std::wstring const &ws) {
            std::wstringstream res{};
            bool escp{false};
            for(std::size_t i = 0; i < ws.size(); ++i) {
                wchar_t c = ws[i];
                if(escp) {
                    switch (c) {
                        case 'b': res << '\b'; break;
                        case 'f': res << '\f'; break;
                        case 'n': res << '\n'; break;
                        case 'r': res << '\r'; break;
                        case 't': res << '\t'; break;
                        default: res << c; break;
                    }
                    escp = false;
                } else {
                    if(c == '\\') {
                        escp = true;
                    } else {
                        res << c;
                    }
                }
            }
            return res.str();
        }

        static std::string unescape_s(std::string const &s) {
            return str_util::to_utf8(wunescape_s(str_util::from_utf8(s)));
        }
    }

    class object_deserializer;

    class object final {
        friend class object_deserializer;

    public:
        typedef std::uint64_t size_type;

    private:
        typedef std::map<std::string, object> o_t;
        typedef std::map<size_type, object> a_t;

    public:
        object() = default;
        object(object const &that): v_{that.v_}, t_{that.t_} {}
        object(object &&that) noexcept: v_{std::move(that.v_)}, t_{that.t_} { that.t_ = jo_null; }
        object(char const *v): v_{(std::string{v})}, t_{jo_string} {}
        object(std::string const &v): v_{(v)}, t_{jo_string} {}
        object(std::string &&v): v_{std::move(v)}, t_{jo_string} {}
        object(std::string_view const &v): v_{(std::string{v})}, t_{jo_string} {}
        object(wchar_t const *v): v_{lins::str_util::to_utf8(v)}, t_{jo_string} {}
        object(std::wstring const &v): v_{lins::str_util::to_utf8(v)}, t_{jo_string} {}
        object(std::wstring_view const &v): v_{lins::str_util::to_utf8((std::wstring{v}))}, t_{jo_string} {}
        object(std::int64_t v): v_{v}, t_{jo_int} {}
        object(std::uint64_t v): v_{(std::int64_t)v}, t_{jo_int} {}
        object(std::int32_t v): v_{(std::int64_t)v}, t_{jo_int} {}
        object(std::uint32_t v): v_{(std::int64_t)v}, t_{jo_int} {}
        object(std::int16_t v): v_{(std::int64_t)v}, t_{jo_int} {}
        object(std::uint16_t v): v_{(std::int64_t)v}, t_{jo_int} {}
        object(std::int8_t v): v_{(std::int64_t)v}, t_{jo_int} {}
        object(std::uint8_t v): v_{(std::int64_t)v}, t_{jo_int} {}
        object(bool v): v_{v}, t_{jo_bool} {}
        object(long double v): v_{v}, t_{jo_flt} {}
        object(float v): v_{(long double)v}, t_{jo_flt} {}
        object(double v): v_{(long double)v}, t_{jo_flt} {}
        object(o_t const &v): v_{v}, t_{jo_object} {}
        object(a_t const &v): v_{v}, t_{jo_array} {}
        object(std::unordered_map<std::string, object> const &v): v_{o_t{}}, t_{jo_object} {
            for(auto &&p: v) {
                v_.as<o_t>()[p.first] = p.second;
            }
        }
        template<typename T>
        object(std::map<std::string, T> const &v): v_{o_t{}}, t_{jo_object} {
            for(auto &&p: v) {
                v_.as<o_t>()[p.first] = p.second;
            }
        }
        object(std::vector<object> const &v): v_{a_t{}}, t_{jo_array} {
            for(std::size_t i{0}; i < v.size(); ++i) {
                v_.as<a_t>()[i] = v[i];
            }
        }
        template<typename T>
        object(std::vector<T> const &v): v_{a_t{}}, t_{jo_array} {
            for(std::size_t i{0}; i < v.size(); ++i) {
                v_.as<a_t>()[i] = v[i];
            }
        }

        object(std::vector<std::uint8_t> const &v): v_{lins::data_to_base64_str(v)}, t_{jo_string} {}

        template<std::size_t N>
        object(std::array<std::uint8_t, N> const &v): v_{lins::data_to_base64_str(v.data(), v.size())}, t_{jo_string} {}

        object(lins::timespec_wrapper const &v): v_{v.to_iso_8601_str()}, t_{jo_string} {}

        object &operator=(const object &that) {
            if(this != &that) {
                v_ = that.v_;
                t_ = (that.t_);
            }
            return *this;
        }

        object &operator=(object &&that) noexcept {
            if(this != &that) {
                std::swap(v_, that.v_);
                std::swap(t_, that.t_);
            }
            return *this;
        }

        object &become_object() {
            if(t_ != jo_object) {
                v_ = o_t{};
                t_ = jo_object;
            }
            return *this;
        }

        object &become_array() {
            if(t_ != jo_array) {
                v_ = a_t{};
                t_ = jo_array;
            }
            return *this;
        }

        object &become_number(std::int64_t val = 0) {
            if(t_ != jo_int) {
                v_ = val;
                t_ = jo_int;
            } else {
                v_ = val;
            }
            return *this;
        }

        object &become_real(long double val = 0.0) {
            if(t_ != jo_flt) {
                v_ = val;
                t_ = jo_flt;
            } else {
                v_ = val;
            }
            return *this;
        }

        object &become_string(std::string const &val = std::string{}) {
            if(t_ != jo_string) {
                v_ = val;
                t_ = jo_string;
            } else {
                v_ = val;
            }
            return *this;
        }

        object &become_boolean(bool val = false) {
            if(t_ != jo_bool) {
                v_ = val;
                t_ = jo_bool;
            } else {
                v_ = val;
            }
            return *this;
        }

        object &become_null() {
            if(t_ != jo_null) {
                v_ = lins::any{};
                t_ = jo_null;
            }
            return *this;
        }

        void reset() {
            become_null();
        }

        bool is_object() const noexcept {
            return t_ == jo_object;
        }

        bool is_array() const noexcept {
            return t_ == jo_array;
        }

        bool is_string() const noexcept {
            return t_ == jo_string;
        }

        bool is_bool() const noexcept {
            return t_ == jo_bool;
        }

        bool is_number() const noexcept {
            return t_ == jo_int;
        }

        bool is_real() const noexcept {
            return t_ == jo_flt;
        }

        bool is_null() const noexcept {
            return t_ == jo_null;
        }

        lins::timespec_wrapper as_date_time() const {
            if(t_ == jo_string) {
                return lins::timespec_wrapper{v_.as<std::string>()};
            } else {
                throw json_error{"invalid source type: string needed"};
            }
        }

        lins::bytevec as_bytevec() const {
            if(is_string()) {
                return lins::base64_str_to_data(lins::any_cast<std::string>(v_));
            } else {
                throw json_error{"invalid source type: string needed"};
            }
        }

        std::int64_t as_number() const {
            if(t_ == jo_int) {
                return lins::any_cast<std::int64_t>(v_);
            } else if(t_ == jo_flt) {
                return lins::any_cast<long double>(v_);
            } else {
                throw json_error{"wrong number"};
            }
        }

        std::int64_t as_number(std::int64_t default_value) const {
            if(t_ == jo_int) {
                return lins::any_cast<std::int64_t>(v_);
            } else if(t_ == jo_flt) {
                return lins::any_cast<long double>(v_);
            } else {
                return default_value;
            }
        }

        int as_int() const {
            if(t_ == jo_int) {
                return lins::any_cast<std::int64_t>(v_);
            } else if(t_ == jo_flt) {
                return lins::any_cast<long double>(v_);
            } else {
                throw json_error{"wrong number"};
            }
        }

        int as_int(int default_value) const {
            if(t_ == jo_int) {
                return lins::any_cast<std::int64_t>(v_);
            } else if(t_ == jo_flt) {
                return lins::any_cast<long double>(v_);
            } else {
                return default_value;
            }
        }

        size_type as_size_t() const {
            if(t_ == jo_flt) {
                return lins::any_cast<long double>(v_);
            } else if(t_ == jo_int) {
                return lins::any_cast<std::int64_t>(v_);
            } else {
                throw json_error{"wrong number"};
            }
        }

        size_type as_size_t(size_type default_value) const {
            if(t_ == jo_flt) {
                return lins::any_cast<long double>(v_);
            } else if(t_ == jo_int) {
                return lins::any_cast<std::int64_t>(v_);
            } else {
                return default_value;
            }
        }

        float as_float() const {
            if(t_ == jo_flt) {
                return lins::any_cast<long double>(v_);
            } else if(t_ == jo_int) {
                return lins::any_cast<std::int64_t>(v_);
            } else {
                throw json_error{"wrong number"};
            }
        }

        float as_float(float default_value) const {
            if(t_ == jo_flt) {
                return lins::any_cast<long double>(v_);
            } else if(t_ == jo_int) {
                return lins::any_cast<std::int64_t>(v_);
            } else {
                return default_value;
            }
        }

        double as_double() const {
            if(t_ == jo_flt) {
                return lins::any_cast<long double>(v_);
            } else if(t_ == jo_int) {
                return lins::any_cast<std::int64_t>(v_);
            } else {
                throw json_error{"wrong number"};
            }
        }

        double as_double(double default_value) const {
            if(t_ == jo_flt) {
                return lins::any_cast<long double>(v_);
            } else if(t_ == jo_int) {
                return lins::any_cast<std::int64_t>(v_);
            } else {
                return default_value;
            }
        }

        long double as_longdouble() const {
            if(t_ == jo_flt) {
                return lins::any_cast<long double>(v_);
            } else if(t_ == jo_int) {
                return lins::any_cast<std::int64_t>(v_);
            } else {
                throw json_error{"wrong number"};
            }
        }

        long double as_longdouble(long double default_value) const {
            if(t_ == jo_flt) {
                return lins::any_cast<long double>(v_);
            } else if(t_ == jo_int) {
                return lins::any_cast<std::int64_t>(v_);
            } else {
                return default_value;
            }
        }

        bool as_boolean() const {
            if(is_bool()) {
                return lins::any_cast<bool>(v_);
            } else if(is_null()) {
                return false;
            } else if(is_number()) {
                return as_number() != 0;
            } else if(is_real()) {
                return as_longdouble() != 0.0L;
            } else if(is_object()) {
                return (lins::any_cast<o_t>(v_)).size() != 0;
            } else if(is_array()) {
                return (lins::any_cast<a_t>(v_)).size() != 0;
            } else if(is_string()) {
                try {
                    long double flt{try_as_long_double()};
                    return flt != 0.0L;
                } catch(...) {
                }
                std::string s{as<std::string>()};
                if(lins::str_util::strtolower(s) == "true") {
                    return true;
                } else if(lins::str_util::strtolower(s) == "false") {
                    return false;
                } else {
                    return s.size() != 0;
                }
            } else {
                throw json_error{"invalid object state"};
            }
        }

        std::string const &string_ref() const {
            if(is_string()) {
                return lins::any_cast<std::string const &>(v_);
            } else {
                throw json_error{"not a string"};
            }
        }

        std::string &string_ref() {
            if(is_string()) {
                return lins::any_cast<std::string &>(v_);
            } else {
                throw json_error{"not a string"};
            }
        }

        std::string as_string() const {
            if(is_null()) {
                return std::string{};
            } else if(is_number()) {
                return convert::itoa(as_number());
            } else if(is_real()) {
                return convert::ftoa(as_longdouble());
            } else if(is_bool()) {
                return as_boolean() ? "true" : "false";
            } else if(is_object()) {
                return serialize();
            } else if(is_array()) {
                return serialize();
            } else if(is_string()) {
                return lins::any_cast<std::string>(v_);
            } else {
                throw json_error{"invalid object state"};
            }
        }

        std::string const &direct_string() const {
            if(is_string()) {
                return v_.as<std::string>();
            } else {
                throw json_error{"json: not a string value"};
            }
        }

        std::string &direct_string() {
            if(is_string()) {
                return v_.as<std::string>();
            } else {
                throw json_error{"json: not a string value"};
            }
        }

        std::wstring as_wstring() const {
            return lins::str_util::from_utf8(as_string());
        }

        size_type size() const {
            if(is_null()) {
                return 0;
            } else if(is_bool()) {
                return sizeof(bool);
            } else if(is_string()) {
                return as_string().size();
            } else if(is_number()) {
                return sizeof(std::int64_t);
            } else if(is_real()) {
                return sizeof(long double);
            } else if(is_object()) {
                return lins::any_cast<o_t>(v_).size();
            } else if(is_array()) {
                const a_t &o{lins::any_cast<a_t &>(v_)};
                if(o.size()) {
                    a_t::const_iterator it{o.end()};
                    --it;
                    size_type a_max{it->first};
                    return a_max + 1;
                } else {
                    return 0;
                }
            }
            throw json_error{"invalid object state"};
        }

        float try_as_float() const {
            if(t_ == jo_flt) {
                return lins::any_cast<long double>(v_);
            } else if(t_ == jo_int) {
                return lins::any_cast<std::int64_t>(v_);
            } else if(t_ == jo_bool) {
                return lins::any_cast<bool>(v_) ? 1 : 0;
            } else if(t_ == jo_null) {
                return 0;
            } else if(t_ == jo_array || t_ == jo_object) {
                return size();
            } else if(t_ == jo_string) {
                return convert::atof(lins::any_cast<std::string>(v_));
            }
            throw json_error{"invalid object state"};
        }

        long double try_as_long_double() const {
            if(t_ == jo_flt) {
                return lins::any_cast<long double>(v_);
            } else if(t_ == jo_int) {
                return lins::any_cast<std::int64_t>(v_);
            } else if(t_ == jo_bool) {
                return lins::any_cast<bool>(v_) ? 1 : 0;
            } else if(t_ == jo_null) {
                return 0;
            } else if(t_ == jo_array || t_ == jo_object) {
                return size();
            } else if(t_ == jo_string) {
                return convert::atof(lins::any_cast<std::string>(v_));
            }
            throw json_error{"invalid object state"};
        }

        double try_as_double() const {
            if(t_ == jo_flt) {
                return lins::any_cast<long double>(v_);
            } else if(t_ == jo_int) {
                return lins::any_cast<std::int64_t>(v_);
            } else if(t_ == jo_bool) {
                return lins::any_cast<bool>(v_) ? 1 : 0;
            } else if(t_ == jo_null) {
                return 0;
            } else if(t_ == jo_array || t_ == jo_object) {
                return size();
            } else if(t_ == jo_string) {
                return convert::atof(lins::any_cast<std::string>(v_));
            }
            throw json_error{"invalid object state"};
        }

        std::int64_t try_as_number() const {
            if(t_ == jo_flt) {
                return lins::any_cast<long double>(v_);
            } else if(t_ == jo_int) {
                return lins::any_cast<std::int64_t>(v_);
            } else if(t_ == jo_bool) {
                return lins::any_cast<bool>(v_) ? 1 : 0;
            } else if(t_ == jo_null) {
                return 0;
            } else if(t_ == jo_array || t_ == jo_object) {
                return size();
            } else if(t_ == jo_string) {
                return try_as_long_double();
            }
            throw json_error{"invalid object state"};
        }

        template<typename T> const T &as() const {
            return lins::any_cast<T const &>(v_);
        }

        template<typename T> T &as() {
            return lins::any_cast<T &>(v_);
        }

        template<typename T>
        std::vector<T> extract_array() const {
            if(!is_array()) {
                throw json_error{"invalid conversion"};
            }
            size_type sz{size()};
            std::vector<T> result{};
            for(size_type i{0}; i < sz; i++) {
                object v{(*this)[i]};
                if(v.is_bool()) {
                    result.push_back((T)v.as_boolean());
                } else if(v.is_null()) {
                    result.push_back((T)0);
                } else if(v.is_number()) {
                    result.push_back((T)v.as_number());
                } else if(v.is_real()) {
                    result.push_back((T)v.as_float());
                } else if(v.is_string()) {
                    result.push_back(lins::any_cast<T>(v.v_));
                } else {
                    throw json_error{"invalid conversion"};
                }
            }
            return result;
        }

        std::vector<std::string> extract_strarray() const {
            if(!is_array()) {
                throw std::runtime_error("invalid conversion");
            }
            size_type sz{size()};
            std::vector<std::string> result{};
            for(size_type i{0}; i < sz; i++) {
                result.push_back((*this)[i].as_string());
            }
            return result;
        }

        object &operator[](std::string const &name) {
            become_object();
            return lins::any_cast<o_t>(v_)[name];
        }

        object const &operator[](std::string const &name) const {
            o_t const &o{lins::any_cast<o_t>(v_)};
            auto it{o.find(name)};
            if(it == o.end()) {
                throw json_error{name + ": field not exist"};
            }
            return it->second;
        }

        object &operator[](std::wstring const &name) {
            return operator[](str_util::to_utf8(name));
        }

        object const &operator[](std::wstring const &name) const {
            return operator[](str_util::to_utf8(name));
        }

        void traverse_object(std::function<void(std::string const &key, object const &val)> const &func) const {
            if(is_object()) {
                o_t const &o{lins::any_cast<o_t>(v_)};
                for(auto &&p: o) {
                    func(p.first, p.second);
                }
            } else {
                throw json_error{"not object"};
            }
        }

        object &operator[](a_t::size_type index) {
            if(is_object()) {
                o_t &o{lins::any_cast<o_t>(v_)};
                o_t::iterator oi{o.begin()};
                for(size_type i{0}; i <= index; ++i) {
                    if(i == index) { return oi->second; }
                    ++oi;
                }
                throw json_error{"index out of range"};
            }
            become_array();
            return lins::any_cast<a_t>(v_)[index];
        }

        object const &operator[](a_t::size_type index) const {
            if(is_object()) {
                o_t const &o = lins::any_cast<o_t>(v_);
                if(o.size() == 0 || index >= o.size()) { throw json_error{"index out of range"}; }
                o_t::const_iterator oi{o.cbegin()};
                for(size_type i{0}; i <= index; ++i) {
                    if(i == index) { return oi->second; }
                    ++oi;
                }
            } else if(is_array()) {
                a_t const &a{lins::any_cast<a_t>(v_)};
                if(a.size() == 0) { throw json_error{"index out of range"}; }
                auto it{a.find(index)};
                if(it != a.end()) {
                    return it->second;
                } else {
                    auto be{a.end()};
                    --be;
                    if(index < be->first) {
                        a_t &am{lins::any_cast<a_t &>(v_)};
                        return am[index];
                    } else {
                        throw json_error{"index out of range"};
                    }
                }
            }
            throw json_error{"indexing is not applicable"};
        }

        void clear() {
            become_null();
        }

        void push_back(object const &v) {
            become_array()[size()] = v;
        }

        void push_back(object &&v) {
            become_array()[size()] = std::move(v);
        }

        static object null_value() {
            return object{};
        }

        bool key_exists(std::string const &key_name) const noexcept {
            try {
                if(!is_object()) { return false; }
                o_t const &o{lins::any_cast<o_t>(v_)};
                return o.find(key_name) != o.end();
            } catch (...) {
            }
            return false;
        }

        bool string_field_exists(std::string const &key_name) const noexcept {
            try {
                if(!is_object()) { return false; }
                o_t const &o{lins::any_cast<o_t>(v_)};
                return o.find(key_name) != o.end() && o.at(key_name).is_string();
            } catch (...) {
            }
            return false;
        }

        bool num_field_exists(std::string const &key_name) const noexcept {
            try {
                if(!is_object()) { return false; }
                o_t const &o{lins::any_cast<o_t>(v_)};
                return o.find(key_name) != o.end() && o.at(key_name).is_number();
            } catch (...) {
            }
            return false;
        }

        bool bool_field_exists(std::string const &key_name) const noexcept {
            try {
                if(!is_object()) { return false; }
                o_t const &o{lins::any_cast<o_t>(v_)};
                return o.find(key_name) != o.end() && o.at(key_name).is_bool();
            } catch (...) {
            }
            return false;
        }

        bool float_field_exists(std::string const &key_name) const noexcept {
            try {
                if(!is_object()) { return false; }
                o_t const &o{lins::any_cast<o_t>(v_)};
                return o.find(key_name) != o.end() && o.at(key_name).is_real();
            } catch (...) {
            }
            return false;
        }

        std::vector<std::string> key_list() const {
            std::vector<std::string> res;
            if(!is_object()) {
                return res;
            }
            for(auto &&p: v_.as<o_t>()) {
                res.push_back(p.first);
            }
            return res;
        }

        std::string key(size_type index) const {
            if(!is_object()) {
                throw json_error{"invalid conversion"};
            }
            o_t const &o = lins::any_cast<o_t>(v_);
            o_t::const_iterator oi = o.begin();
            for(size_type i = 0; i <= index; i++) {
                if(i == index) {
                    return oi->first;
                }
                ++oi;
            }
            throw json_error{"index out of range"};
        }

        void key_remove(size_type index) {
            if(!is_object()) {
                throw json_error{"invalid conversion"};
            }
            std::string name = this->key(index);
            o_t &o{lins::any_cast<o_t>(v_)};
            o_t::iterator oi = o.find(name);
            if(oi != o.end()) o.erase(oi);
        }

        void key_remove(std::string const &name) {
            if(!is_object()) {
                throw json_error{"invalid conversion"};
            }
            o_t &o{lins::any_cast<o_t>(v_)};
            o_t::iterator oi = o.find(name);
            if(oi != o.end()) o.erase(oi);
        }

        void array_item_remove(size_type index) {
            if(!is_array()) {
                throw json_error{"invalid conversion"};
            }
            a_t &a{lins::any_cast<a_t>(v_)};
            a_t::iterator ai = a.find(index);
            if(ai != a.end()) a.erase(ai);
            size_type s = size();
            for(size_type i = index + 1; i < s; i++) {
                a_t::iterator ai = a.find(i);
                if(ai != a.end()) {
                    a[i - 1] = ai->second;
                    a.erase(ai);
                }
            }
        }

        void erase(size_type index) {
            if(is_array()) {
                array_item_remove(index);
            } else if(is_object()) {
                key_remove(index);
            } else {
                throw json_error{"invalid conversion"};
            }
        }

        void erase(std::string const &key) {
            if(is_object()) {
                key_remove(key);
            } else {
                throw json_error{"invalid conversion"};
            }
        }

        object &parse(std::string const &s) {
            *this = deserialize(s);
            return *this;
        }

        object &parse(std::string_view const &s) {
            *this = deserialize(s);
            return *this;
        }

        object &parse(const char *str, std::size_t siz) {
            *this = deserialize(std::string_view{str, siz});
            return *this;
        }

        static object deserialize(std::string const &s, bool use_hw_acclr = true);

        static object deserialize(std::string_view const &s ,bool use_hw_acclr = true);

        static object deserialize(char const *s ,bool use_hw_acclr = true);

        std::string serialize(bool compact = true) const {
            return serialize_actual(compact, 0);
        }

//        operator std::string() const {
//            return serialize();
//        }

//        operator std::int64_t() const {
//            return as_number(0);
//        }

//        operator long double() const {
//            return as_longdouble(0);
//        }

//        operator double() const {
//            return as_double(0);
//        }

//        operator float() const {
//            return as_float(0);
//        }

//        operator int() const {
//            return as_int(0);
//        }

//        operator size_type() const {
//            return as_size_t(0);
//        }

        void load_from_file(std::string const &file_name) {
            become_null();
            std::ifstream ifs;
            ifs.open(file_name.c_str());
            std::stringstream str_stream;
            if(ifs.good()) {
                std::string buf;
                while(std::getline(ifs, buf)) {
                    str_stream << buf;
                }
            }
            parse(str_stream.str());
        }

        void save_to_file(std::string const &file_name) const {
            std::ofstream ofs;
            ofs.open(file_name.c_str());
            if(ofs.good()) {
                ofs << serialize(false);
                ofs.close();
            }
        }

    private:
        std::int64_t max_array_index() const {
            if(v_.as<a_t const &>().size() == 0) {
                return -1;
            } else {
                auto it = v_.as<a_t const &>().end();
                --it;
                return it->first;
            }
        }

        std::string serialize_actual(bool compact, int level = 0) const {
            std::string newline{compact ? "" : "\n"};
            std::string indent{compact ? "" : "    "};
            std::string space{compact ? "" : " "};
            std::string prefix{};
            for(int l{0}; l < level; ++l) { prefix += indent; }

            if(t_ == jo_null) {
                return "null";
            } else if(t_ == jo_int) {
                return lins::str_util::itoa(v_.as<std::int64_t>());
            } else if(t_ == jo_flt) {
                return convert::ftoa(v_.as<long double>());
            } else if(t_ == jo_bool) {
                return v_.as<bool>() ? "true" : "false";
            } else if(t_ == jo_string) {
                return std::string{ "\"" } + convert::escape(v_.as<std::string>()) + std::string{ "\"" };
            } else if(t_ == jo_object) {
                std::string res{std::string{ "{" } + (v_.as<o_t>().size() ? newline : "")};
                std::string comma{ "," };
                std::size_t i{0};
                for(auto &&p: v_.as<o_t const &>()) {
                    if(++i >= v_.as<o_t const &>().size()) { comma = ""; }
                    res += prefix + indent + std::string{ "\"" } + p.first + "\"";
                    res += std::string() + ":" + space;
                    res += p.second.serialize_actual(compact, level + 1);
                    res += comma + (i < v_.as<o_t>().size() ? newline : "");
                }
                res += (i > 0 ? newline + prefix : "") + "}";
                return res;
            } else if(t_ == jo_array) {
                std::string res{std::string{"["} + (v_.as<a_t>().size() ? newline : "")};
                std::string comma{ "," };
                std::int64_t max_ind{max_array_index()};
                std::int64_t i{0};
                for(; i <= max_ind; ++i) {
                    if(i >= max_ind) { comma = ""; }
                    res += prefix + indent;
                    a_t::const_iterator it{v_.as<a_t>().find(i)};
                    if(it != v_.as<a_t>().end()) {
                        auto &&p{it->second};
                        res += p.serialize_actual(compact, level + 1);
                    } else {
                        res += "null";
                    }
                    res += comma + (i - 1 < max_ind ? newline : "");
                }
                res += (i > 0 ? prefix : "") + "]";
                return res;
            } else {
                throw json_error{"invalid object"};
            }
        }

    private:
        enum type {
            jo_null,
            jo_object,
            jo_array,
            jo_string,
            jo_int,
            jo_flt,
            jo_bool
        };

        lins::any v_{};
        type t_{jo_null};
    };


    class object_deserializer final {
    public:
        object_deserializer() {
            reset();
        }

        void reset() {
            stack_.clear();
            stack_.push_back({});
            what_next_ = expected::listed;
            next_list_.clear();
            next_list_ << "str" << "sts" << "int" << "fpl" << "tru" << "fal" << "nul" << "[" << "{";
        }

        auto harvest_result() {
            return std::move(top().obj);
        }

        void any_value_observer_accept_value(lins::any const &a) {
            std::map<std::string, lins::any> &o = a.as<std::map<std::string, lins::any> &>();
            auto t = o["type"].as<std::string>().substr(1);
            if(t != "spc") {
                auto k = o["token"].as<std::string>();
                if(t == "str" || t == "sts") {
                    k = k.substr(1, k.size() - 2);
                }
                consume_token(k, t, o["line"].as<int>(), o["column"].as<int>());
            }
        }

        void consume_token(std::string const &tk, std::string const &tt, int line, int col) {
            if(tt == "//") {
                return;
            }
            if(
                what_next_ == expected::none
                || (what_next_ == expected::listed && !next_list_.contains(tt))
                || (stack_.size() == 1 && top().closed)
            ) {
                static const std::map<std::string, std::string> exp_map {
                    {"str" , "<string>" },
                    {"sts" , "<string>" },
                    {"int" , "<int>" },
                    {"fpl" , "<number>" },
                    {"tru" , "\"true\"" },
                    {"fal" , "\"false\"" },
                    {"nul" , "\"null\"" },
                    {"["   , "\"[\"" },
                    {"]"   , "\"]\"" },
                    {"{"   , "\"{\"" },
                    {"}"   , "\"}\"" },
                    {","   , "\",\"" },
                    {":"   , "\":\"" },
                };
                std::stringstream ss{};
                ss << "at <" << line << ":" << col << "> expected: ";
                if(what_next_ == expected::listed) {
                    std::string sep{};
                    for(auto&& s: next_list_.elements()) {
                        ss << sep << exp_map.at(s);
                        sep = " or ";
                    }
                } else if(what_next_ == expected::none) {
                    ss << "Nothing";
                } else if(what_next_ == expected::any) {
                    ss << "Anything";
                } else {
                    ss << "<Unknown>";
                }
                ss << ", got: \"" << tk << "\" (" << exp_map.at(tt) << ")";
                throw json_error{ss.str()};
            }

            if(tt == "str" || tt == "sts") {
                if(top().obj.is_array()) {
                    top().obj.push_back(tt == "str" ? convert::unescape(tk) : convert::unescape_s(tk));
                    what_next_ = expected::listed;
                    next_list_.clear();
                    next_list_ << "," << "]";
                } else if(top().obj.is_object()) {
                    push();
                    top().name = tt == "str" ? convert::unescape(tk) : convert::unescape_s(tk);
                    ++top().obj_itm_phase;
                    what_next_ = expected::listed;
                    next_list_.clear();
                    next_list_ << ":";
                } else if(top().obj_itm_phase == 2) {
                    top().obj = tt == "str" ? convert::unescape(tk) : convert::unescape_s(tk);
                    ++top().obj_itm_phase;
                    what_next_ = expected::listed;
                    next_list_.clear();
                    next_list_ << "}" << "]" << ",";
                    top().closed = true;
                } else {
                    top().obj = tt == "str" ? convert::unescape(tk) : convert::unescape_s(tk);
                    what_next_ = expected::none;
                    top().closed = true;
                }
            } else if(tt == "int") {
                if(top().obj.is_array()) {
                    top().obj.push_back(lins::str_util::atoi(tk));
                    what_next_ = expected::listed;
                    next_list_.clear();
                    next_list_ << "," << "]";
                } else if(top().obj_itm_phase == 2) {
                    top().obj = lins::str_util::atoi(tk);
                    ++top().obj_itm_phase;
                    what_next_ = expected::listed;
                    next_list_.clear();
                    next_list_ << "}" << "]" << ",";
                    top().closed = true;
                } else {
                    top().obj = lins::str_util::atoi(tk);
                    what_next_ = expected::none;
                    top().closed = true;
                }
            } else if(tt == "fpl") {
                if(top().obj.is_array()) {
                    std::istringstream os(tk); long double ld; os >> ld;
                    top().obj.push_back(ld);
                    what_next_ = expected::listed;
                    next_list_.clear();
                    next_list_ << "," << "]";
                } else if(top().obj_itm_phase == 2) {
                    std::istringstream os(tk); long double ld; os >> ld;
                    top().obj = ld;
                    ++top().obj_itm_phase;
                    what_next_ = expected::listed;
                    next_list_.clear();
                    next_list_ << "}" << "]" << ",";
                    top().closed = true;
                } else {
                    std::istringstream os(tk); long double ld; os >> ld;
                    top().obj = ld;
                    what_next_ = expected::none;
                    top().closed = true;
                }
            } else if(tt == "nul") {
                if(top().obj.is_array()) {
                    top().obj.push_back({});
                    what_next_ = expected::listed;
                    next_list_.clear();
                    next_list_ << "," << "]";
                } else if(top().obj_itm_phase == 2) {
                    top().obj = {};
                    ++top().obj_itm_phase;
                    what_next_ = expected::listed;
                    next_list_.clear();
                    next_list_ << "}" << "]" << ",";
                    top().closed = true;
                } else {
                    top().obj = {};
                    what_next_ = expected::none;
                    top().closed = true;
                }
            } else if(tt == "tru" || tt == "fal") {
                if(top().obj.is_array()) {
                    top().obj.push_back(tk.size() == 4 ? true : false);
                    what_next_ = expected::listed;
                    next_list_.clear();
                    next_list_ << "," << "]";
                } else if(top().obj_itm_phase == 2) {
                    top().obj = tk.size() == 4 ? true : false;
                    ++top().obj_itm_phase;
                    what_next_ = expected::listed;
                    next_list_.clear();
                    next_list_ << "}" << "]" << ",";
                    top().closed = true;
                } else {
                    top().obj = tk.size() == 4 ? true : false;
                    what_next_ = expected::none;
                    top().closed = true;
                }
            } else if(tt == "{") {
                if(top().obj_itm_phase == 2) {
                    ++top().obj_itm_phase;
                    top().closed = true;
                }
                push();
                top().obj.become_object();
                what_next_ = expected::listed;
                next_list_.clear();
                next_list_ << "str" << "sts" << "}";
            } else if(tt == "}") {
                if(!top().obj.is_object()) {
                    throw json_error{ "\"}\" - invalid token." };
                }
                top().closed = true;
                if(stack_.size() == 2 && top().closed) {
                    what_next_ = expected::none;
                } else {
                    what_next_ = expected::listed;
                    next_list_.clear();
                    next_list_ << "]" << "}" << ",";
                }
            }
            else if(tt == "[") {
                if(top().obj_itm_phase == 2) {
                    ++top().obj_itm_phase;
                    top().closed = true;
                }
                push();
                top().obj.become_array();
                what_next_ = expected::listed;
                next_list_.clear();
                next_list_ << "str" << "sts" << "int" << "fpl" << "tru" << "fal" << "nul" << "[" << "]" << "{";
            } else if(tt == "]") {
                if(!top().obj.is_array()) {
                    throw json_error{ "\"]\" - invalid token." };
                }
                top().closed = true;

                if(stack_.size() == 2 && top().closed) {
                    what_next_ = expected::none;
                } else {
                    what_next_ = expected::listed;
                    next_list_.clear();
                    next_list_ << "]" << "}" << ",";
                }
            } else if(tt == ":") {
                if(top().obj_itm_phase != 1) {
                    throw json_error{ "\":\" - invalid token, name expected." };
                }
                ++top().obj_itm_phase;
                what_next_ = expected::listed;
                next_list_.clear();
                next_list_ << "str" << "sts" << "int" << "fpl" << "tru" << "fal" << "nul" << "[" << "{";
            }
            else if(tt == ",") {
                if(!top().obj.is_object() && !top().obj.is_array()) {
                    throw json_error{ "\":\" - invalid token, name expected." };
                }

                if(top().obj.is_object()) {
                    what_next_ = expected::listed;
                    next_list_.clear();
                    next_list_ << "str" << "sts" << "}";
                } else if(top().obj.is_array()) {
                    what_next_ = expected::listed;
                    next_list_.clear();
                    next_list_ << "str" << "sts" << "int" << "fpl" << "tru" << "fal" << "nul" << "[" << "]" << "{";
                }
            } else {
                throw json_error{std::string{"invalid token: \""} + tk + "\""};
            }
            collapse_top_down();
        }

    private:
        struct stack_item {
            object obj{};
            std::string name{};
            int obj_itm_phase{ 0 };
            bool closed{ false };
        };

        std::list<stack_item> stack_;

        enum class expected {
            any,
            none,
            listed
        };

        expected what_next_{ expected::any };
        lins::set_of<std::string> next_list_{};

    private:
        inline stack_item &top() {
            return stack_.back();
        }

        inline stack_item &pre_top() {
            auto it{stack_.end()};
            --it; --it;
            return *it;
        }

        inline void pop() {
            stack_.pop_back();
        }

        inline void push() {
            stack_.push_back({ {}, {}, 0, false });
        }

        void collapse_top_down() {
            while(stack_.size() > 1 && top().closed) {
                if(pre_top().obj.is_object() && top().obj_itm_phase == 3) {
                    object o{std::move(top().obj)};
                    std::string n{ std::move(top().name) };
                    pop();
                    top().obj[n] = std::move(o);
                } else if(pre_top().obj.is_array()) {
                    object o{std::move(top().obj)};
                    std::string n{ std::move(top().name) };
                    pop();
                    top().obj.push_back(std::move(o));
                } else {
                    pre_top().obj = std::move(top().obj);
                    pop();
                }
                if(stack_.size() == 1) {
                    top().closed = true;
                }
            }
        }
    };
}
