#pragma once

#include "commondefs.hpp"
#ifdef PLATFORM_WINDOWS
#include <wctype.h>
#include <wchar.h>
#endif

extern char *unicode_data[];

namespace lins {

    namespace str_util {

        template<class T>
        std::string binstr(T d) {
            std::string s;
            while(d != 0) {
                s = (d & 1 ? std::string("1") : std::string("0")) + s;
                d >>= 1;
            }
            return s;
        }

        template<class T>
        T strbin(const std::string &s) {
            T res = 0;
            for(int i = 0; i < (int)s.size(); i++) {
                T t = 1;
                res |= (s[i] - '0') ? t << (s.size() - i - 1): 0;
            }
            return res;
        }



        template<class T>
        std::vector<std::uint8_t> to_bytes(T d) {
            std::vector<std::uint8_t> res{(std::uint8_t *)&d, (std::uint8_t *)&d + sizeof(T)};
            return res;
        }

        template<class T>
        T from_bytes(std::vector<std::uint8_t> const &v) {
            T res{};
            if(sizeof(T) != v.size()) {
                throw std::runtime_error{"from_bytes(): type size does not match"};
            }
            std::memcpy(&res, v.data(), sizeof(T));
            return res;
        }


        std::string prepend(const std::string &what, char with, std::size_t up_to_size);

        std::string ltrim(const std::string &s);
        std::string rtrim(const std::string &s);
        std::string trim(const std::string &s);

        std::wstring ltrim(const std::wstring &s);
        std::wstring rtrim(const std::wstring &s);
        std::wstring trim(const std::wstring &s);

        template<typename string_t>
        void replace_substring_inplace(string_t &where, const string_t &what, const string_t &to) {
            std::size_t pos{0};
            while(true) {
                pos = where.find(what, pos);
                if(pos == string_t::npos) {
                    break;
                }
                where.replace(pos, what.size(), to);
                pos += to.size();
            }
        }

        template<typename string_t>
        string_t replace_substring(const string_t &where, const string_t &what, const string_t &to) {
            string_t where1{where};
            replace_substring_inplace(where1, what, to);
            return where1;
        }

        template<typename C>
        std::string container_to_string(const C &v) {
            std::string res;
            for(const auto &s: v) {
                res += s + "\n";
            }
            return res;
        }

        template<typename STR_T>
        STR_T unquote(const STR_T &s, const STR_T &l, const STR_T &r) {
            STR_T res;
            if(s.size() >= l.size() + r.size()) {
                if(s.substr(0, l.size()) == l &&
                   s.substr(s.size() - r.size()) == r
                ) {
                    res = s.substr(l.size(), s.size() - l.size() - r.size());
                }
            }
            return res;
        }

        template<typename STR_T>
        bool string_starts_from(const STR_T &str, const STR_T &from) {
            return (str.empty() && from.empty())
                    ||
                   (str.size() >= from.size()
                    &&
                    str.substr(0, from.size()) == from);
        }

        template<typename STR_T>
        class unescaper {
            /*
            Control characters:

            (Hex codes assume an ASCII-compatible character encoding.)

                \a = \x07 = alert (bell)
                \b = \x08 = backspace
                \t = \x09 = horizonal tab
                \n = \x0A = newline (or line feed)
                \v = \x0B = vertical tab
                \f = \x0C = form feed
                \r = \x0D = carriage return
                \e = \x1B = escape (non-standard GCC extension)

            Punctuation characters:

                \" = quotation mark (backslash not required for '"')
                \' = apostrophe (backslash not required for "'")
                \? = question mark (used to avoid trigraphs)
                \\ = backslash

            Numeric character references:

                \ + up to 3 octal digits
                \x + any number of hex digits
                \u + 4 hex digits (Unicode BMP, new in C++11)
                \U + 8 hex digits (Unicode astral planes, new in C++11)

            \0 = \00 = \000 = octal ecape for null character

            If you do want an actual digit character after a \0, then yes, I recommend string concatenation.
            Note that the whitespace between the parts of the literal is optional, so you can write "\0""0".
            */
        public:
            unescaper(const STR_T &str = STR_T()) {
                unescape(str);
            }

            unescaper &unescape(const STR_T &str) {
                res.clear();
                esc = escape::no;
                for(auto c: str) {
                    unescape_char(c);
                }
                unescape_char(0);
                return *this;
            }

            operator STR_T() {
                return res;
            }

        private:
            STR_T res;
            enum class escape { no, start, oct1, oct2, hex, uni4, uni8 };
            escape esc = escape::no;
            std::string cur_char_num;

            void unescape_char(typename STR_T::value_type cc) {
                switch(esc) {
                    case escape::no:
                        if(cc == '\\') {
                            esc = escape::start;
                            cur_char_num.clear();
                        } else if(cc != 0) {
                            res += cc;
                        }
                        break;
                    case escape::start:
                        if(cc == 0) {
                            throw std::runtime_error("invalid escape");
                        }
                        else if(cc == 'r') { cc = '\r'; res += cc; esc = escape::no; }
                        else if(cc == 'n') { cc = '\n'; res += cc; esc = escape::no; }
                        else if(cc == 't') { cc = '\t'; res += cc; esc = escape::no; }
                        else if(cc == 'b') { cc = '\b'; res += cc; esc = escape::no; }
                        else if(cc == 'a') { cc = '\a'; res += cc; esc = escape::no; }
                        else if(cc == 'v') { cc = '\v'; res += cc; esc = escape::no; }
                        else if(cc == 'f') { cc = '\f'; res += cc; esc = escape::no; }
                        else if(cc == 'e') { cc = 0x1b; res += cc; esc = escape::no; }
                        else if(cc >= '0' && cc <= '7') { esc = escape::oct1; cur_char_num += cc; }
                        else if(cc == 'x') { esc = escape::hex; }
                        else if(cc == 'u') { esc = escape::uni4; }
                        else if(cc == 'U') { esc = escape::uni8; }
                        else if(cc == '\\') { res += cc; esc = escape::no; }
                        else { esc = escape::no; unescape_char(cc); }
                        break;
                    case escape::oct1:
                        if(cc != 0 && cc >= '0' && cc <= '7') {
                            esc = escape::oct2;
                            cur_char_num += cc;
                        } else {
                            char *end;
                            res += (typename STR_T::value_type)::strtol(cur_char_num.c_str(), &end, 8);
                            esc = escape::no;
                            unescape_char(cc);
                        }
                        break;
                    case escape::oct2:
                        esc = escape::no;
                        if(cc != 0 && cc >= '0' && cc <= '7') {
                            cur_char_num += cc;
                            char *end;
                            res += (typename STR_T::value_type)::strtol(cur_char_num.c_str(), &end, 8);
                        } else {
                            char *end;
                            res += (typename STR_T::value_type)::strtol(cur_char_num.c_str(), &end, 8);
                            unescape_char(cc);
                        }
                        break;
                    case escape::hex:
                        if(cc != 0 && ((cc >= '0' && cc <= '9') || (cc >= 'a' && cc <= 'f') || (cc >= 'A' && cc <= 'F'))) {
                            cur_char_num += cc;
                        } else {
                            char *end;
                            res += (typename STR_T::value_type)::strtol(cur_char_num.c_str(), &end, 16);
                            esc = escape::no;
                            unescape_char(cc);
                        }
                        break;
                    case escape::uni4:
                        if(cc != 0 && ((cc >= '0' && cc <= '9') || (cc >= 'a' && cc <= 'f') || (cc >= 'A' && cc <= 'F'))) {
                            cur_char_num += cc;
                            if(cur_char_num.size() == 4) {
                                char *end;
                                res += (typename STR_T::value_type)::strtol(cur_char_num.c_str(), &end, 16);
                                esc = escape::no;
                            }
                        } else {
                            if(cur_char_num.size() != 4) {
                                throw std::runtime_error("unicode sequence is not completed");
                            }
                            char *end;
                            res += (typename STR_T::value_type)::strtol(cur_char_num.c_str(), &end, 16);
                            esc = escape::no;
                            unescape_char(cc);
                        }
                        break;
                    case escape::uni8:
                        if(cc != 0 && ((cc >= '0' && cc <= '9') || (cc >= 'a' && cc <= 'f') || (cc >= 'A' && cc <= 'F'))) {
                            cur_char_num += cc;
                            if(cur_char_num.size() == 8) {
                                char *end;
                                res += (typename STR_T::value_type)::strtol(cur_char_num.c_str(), &end, 16);
                                esc = escape::no;
                            }
                        } else {
                            if(cur_char_num.size() != 8) {
                                throw std::runtime_error("unicode sequence is not completed");
                            }
                            char *end;
                            res += (typename STR_T::value_type)::strtol(cur_char_num.c_str(), &end, 16);
                            esc = escape::no;
                            unescape_char(cc);
                        }
                        break;
                }
            }
        };

        int toupper(int);
        int tolower(int);
        int islower(int);
        int isalpha(int);
        int isalnum(int);
        int isdigit(int);
        int isprint(int);
        int isascii_(int);
        int iscntrl(int);
        int ispunct(int);
        int isspace(int);

        int towupper(int);
        int towlower(int);
        int iswlower(int);
        int iswalpha(int);
        int iswalnum(int);
        int iswdigit(int);
        int iswprint(int);
        int iswascii_(int);
        int iswcntrl(int);
        int iswpunct(int);
        int iswspace(int);

        std::uint64_t utf8_to_ucs(char const *utf8, int *increment = nullptr);
        int ucs_to_utf8(std::uint32_t c, unsigned char *utf8);

        std::string strtolower(const std::string &);
        std::string strtoupper(const std::string &);

        std::wstring wstrtolower(const std::wstring &);
        std::wstring wstrtoupper(const std::wstring &);

        std::string to_utf8(const std::wstring &str);
        //std::string to_utf8(const wchar_t *str);
        template<typename char_type>
        std::string to_utf8(const char_type *str) {
            std::list<char> result;
            for(const char_type *wi = str; wi && *wi; ++wi) {
                char buf[20];
                int s = ucs_to_utf8(*wi, (unsigned char *)buf);
                for(int j = 0; j < s; j++) {
                    result.push_back(buf[j]);
                }
            }
            return std::string(result.begin(), result.end());
        }
        std::wstring from_utf8(const std::string &s);
        std::wstring from_utf8(const char *s);
        void to_utf8(const std::wstring &str, std::string &result);
        //void to_utf8(const wchar_t *str, std::string &result);

        template<typename char_type>
        void to_utf8(const char_type *str, std::string &result) {
            std::stringstream ss;
            for(const char_type *wi = str; *wi; ++wi) {
                char buf[20];
                ucs_to_utf8(*wi, (unsigned char *)buf);
                ss << buf;
            }
            result = ss.str();
        }

        void from_utf8(const std::string &s, std::wstring &result);
        void from_utf8(const char *s, std::wstring &result);

        std::string itoa(std::int64_t value, int radix = 10);
        std::string utoa(std::uint64_t value, int radix = 10);
        //lins::int64_t atoi(const std::string &a, int radix = 10);

        template<typename S_T>
        std::int64_t atoi(const S_T &a, int radix = 10) {
            static int const digits[256] = {
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1,
                -1,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,
                25,26,27,28,29,30,31,32,33,34,35,-1,-1,-1,-1,-1,
                -1,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,
                25,26,27,28,29,30,31,32,33,34,35,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
            };

            if(!a.size()) {
                throw std::runtime_error{"empty string passed"};
            }
            if((radix < 2) || (radix > 36)) {
                throw std::runtime_error{"invalid radix"};
            }

            bool sign_set = false;
            bool negative = false;
            size_t str_size = a.size();
            std::int64_t result = 0;
            std::int64_t pwr = 1;
            bool allright{true};
            for(int i = (int)str_size - 1; i >= 0; i--, pwr *= radix) {
                if(a[i] == '-') {
                    if(sign_set) { allright = false; break; }
                    sign_set = true;
                    negative = true;
                    break;
                }
                if(a[i] == '+') {
                    if(sign_set) { allright = false; break; }
                    sign_set = true;
                    negative = false;
                    break;
                }
                int d = (int) a[i];
                if(d > 255) { allright = false; break; }
                int dd = digits[d];
                if((dd < radix) && (dd >= 0)) {
                    result += dd * pwr;
                } else { allright = false; break; }
            }
            if(!allright) {
                throw std::runtime_error{"invalid string passed"};
            }
            return negative ? -result : result;
        }


        static inline std::int64_t atoi(char const *a, int radix = 10) {
            return atoi<std::string>(a, radix);
        }

        static inline std::int64_t atoi(wchar_t const *a, int radix = 10) {
            return atoi<std::wstring>(a, radix);
        }

        std::uint64_t atoui(const std::string &a, int radix = 10);

        bool check_only_mentioned(const std::string &a, const std::string &chars);
        std::string leave_only_mentioned(const std::string &a, const std::string &chars);
        real atonum(const std::string &a);
        bool str_to_bool(const std::string &s);
        template<typename T>
        static char first_digit_after_point(T fpd) {
            static constexpr char digits[]{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
            char res{'0'};
            T fpda{std::abs(fpd)};
            T i{1}; T f{std::modf(fpda, &i)};
            f *= 10;
            T fc{std::ceil(f)};
            T ff{std::floor(f)};
            if(std::abs(f - fc) < 1e-6) {
                res = digits[(int)fc];
            } else if(std::abs(f - ff) < 1e-6) {
                res = digits[(int)ff];
            } else {
                res = digits[(int)f];
            }
            return res;
        }

        template<typename T>
        std::string ftoa(T val) {
            long double av{std::abs((long double )val)};
            long double ipart{};
            long double fpart{modfl(av, &ipart)};
            static auto border_val{
                [](long double val, long double &res, long double eps) -> bool {
                    long double ival{};
                    modfl(val, &ival);
                    if(std::abs(ival - val) < eps) {
                        res = ival;
                        return true;
                    } else if(std::abs(ival + 1 - val) < eps) {
                        res = ival + 1;
                        return true;
                    }
                    return false;
                }
            };
            static auto last_digit{
                [](long double val, long double eps) -> char {
                    long double val_div_10{val * 0.1L};
                    long double ipart{};
                    long double fpart{modfl(val_div_10, &ipart)};

                    long double fd{fpart * 10.0L};
                    long double fda{};
                    int ld{(int)(fd)};
                    if(border_val(fd, fda, eps)) {
                        ld = fda;
                    }
                    return '0' + ld;
                }
            };
            std::string fs{};
            long double feps{std::numeric_limits<long double>::min()};
            bool feps_softened{false};
            for(int iter{0}; ; ++iter) {
                if(fpart < feps) {
                    break;
                } else if(1 - fpart < feps) {
                    if(iter) {
                            fs += '1';
                            if(!feps_softened) {
                                feps = 1e-4;
                                feps_softened = true;
                            }
                    } else {
                            ipart += 1;
                    }
                    break;
                } else {
                    fpart *= 10;
                    long double bv{};
                    if(border_val(fpart, bv, feps)) {
                            int ifr{(int)bv};
                            fs += ifr + '0';
                            break;
                    }
                    int ifr{(int)fpart};
                    if(ifr && !feps_softened) {
                            feps = 1e-4;
                            feps_softened = true;
                    }
                    fs += ifr + '0';
                    fpart -= ifr;
                }
            }
            std::string is{};
            while(ipart >= 1) {
                is.push_back(last_digit(ipart, 1e-5));
                ipart /= 10.0L;
            }
            int issize{(int)(is.size())};
            int issize_2{issize / 2};
            for(int j{0}; j < issize_2; ++j) {
                std::swap(is[j], is[issize - 1 - j]);
            }

            if(is.empty()) { is = "0"; }
            if(fs.empty()) { fs = "0"; }

            return (val < 0 ? std::string{"-"} : std::string{}) + is + '.' + fs;
        }

        std::string data_to_hex_str(const void *data, int data_size);
        std::string data_to_hex_str(const std::vector<std::uint8_t> &data);
        std::string data_to_hex_str(const std::string &data);
        template<std::size_t ARRAY_SIZE>
        std::string data_to_hex_str(const std::array<std::uint8_t, ARRAY_SIZE> &data) {
            return data_to_hex_str(data.data(), data.size());
        }
        std::vector<std::uint8_t> hex_str_to_data(const std::string &str);

        std::string data_to_base64_str(const void *data, std::size_t data_size);
        std::string data_to_base64_str(std::vector<std::uint8_t> const &data);
        std::vector<std::uint8_t> base64_str_to_data(const std::string &);
        template<std::size_t ARRAY_SIZE>
        std::string data_to_base64_str(const std::array<std::uint8_t, ARRAY_SIZE> &data) {
            return data_to_base64_str(data.data(), data.size());
        }
        bool is_valid_base64(std::string const &);

        template<typename string_class>
        std::vector<string_class> str_tok(const string_class &str, const string_class &separator, bool separator_entire_str = true) {
            std::vector<string_class> result{};
            typename string_class::size_type begin_pos = 0;
            typename string_class::size_type end_pos = 0;
            while(begin_pos <= str.size()) {
                end_pos = separator_entire_str ? str.find(separator, begin_pos) : str.find_first_of(separator, begin_pos);
                if(end_pos != string_class::npos) {
                    result.push_back(str.substr(begin_pos, end_pos - begin_pos));
                } else {
                    result.push_back(str.substr(begin_pos));
                    break;
                }
                begin_pos = separator_entire_str ? (end_pos + separator.size()) : (end_pos + 1);
            }

            return result;
        }

        template<typename string_class>
        std::vector<string_class> str_tok(const string_class &str, std::function<bool(std::uint64_t)>comp_func) {
            std::vector<string_class> result;
            typename string_class::size_type begin_pos = 0;
            typename string_class::size_type end_pos = 0;
            while(end_pos < str.size()) {
                if(comp_func(str[end_pos])) {
                    result.push_back(str.substr(begin_pos, end_pos - begin_pos));
                    end_pos = begin_pos = end_pos + 1;
                } else {
                    ++end_pos;
                }
            }
            result.push_back(str.substr(begin_pos, end_pos - begin_pos));
            return result;
        }

        template<typename IT>
        std::string stringify_range(IT b, IT e, const std::string &left_br = "[", const std::string &right_br = "]") {
            std::ostringstream oss;
            std::string sep{""};
            oss << left_br;
            for(IT it{b}; it != e; ++it) {
                oss << sep << *it;
                sep = ", ";
            }
            oss << right_br;
            return oss.str();
        }

        std::string hexdump(const void *d, std::size_t s, std::size_t cols = 16, const std::string &splitter = std::string("  "), bool control_dots = true);
        std::string hexdump(const std::vector<std::uint8_t> &d, std::size_t cols = 16, const std::string &splitter = std::string("  "), bool control_dots = true);
        std::string hexdump(const std::deque<std::uint8_t> &d, std::size_t cols = 16, const std::string &splitter = std::string("  "), bool control_dots = true);
        std::string hexdump(const std::string &d, std::size_t cols = 16, const std::string &splitter = std::string("  "), bool control_dots = true);
        template<std::size_t ARRAY_SIZE>
        std::string hexdump(const std::array<std::uint8_t, ARRAY_SIZE> &d, std::size_t cols = 16, const std::string &splitter = std::string("  "), bool control_dots = true) {
            return hexdump(d.data(), d.size(), cols, splitter, control_dots);
        }

        #if defined(UNICODE) || defined(_UNICODE)

        #define tchar_t wchar_t
        typedef std::wstring tstring;
        #define tcout std::wcout
        #define tcin std::wcin

        #ifndef __T
        #define __T(x)      L ## x
        #endif

        #define totupper     str_util::towupper
        #define totlower	 str_util::towlower
        #define istlower	 str_util::iswlower
        #define istalpha	 str_util::iswalpha
        #define istalnum	 str_util::iswalnum
        #define istdigit	 str_util::iswdigit
        #define istprint	 str_util::iswprint
        #define istascii	 str_util::iswascii_
        #define istcntrl	 str_util::iswcntrl
        #define istpunct	 str_util::iswpunct
        #define istspace	 str_util::iswspace
        #define tstrtolower	 str_util::wstrtolower
        #define tstrtoupper	 str_util::wstrtoupper

        #else


        #ifndef __T
        #define __T(x)      x
        #endif


        #define tchar_t char
        typedef std::string tstring;
        #define tcout std::cout
        #define tcin std::cin

        #define totupper     str_util::toupper
        #define totlower	 str_util::tolower
        #define istlower	 str_util::islower
        #define istalpha	 str_util::isalpha
        #define istalnum	 str_util::isalnum
        #define istdigit	 str_util::isdigit
        #define istprint	 str_util::isprint
        #define istascii	 str_util::isascii_
        #define istcntrl	 str_util::iscntrl
        #define istpunct	 str_util::ispunct
        #define istspace	 str_util::isspace
        #define tstrtolower	 str_util::strtolower
        #define tstrtoupper	 str_util::strtoupper

        #endif


        #define _T(x)       __T(x)
        #define _TEXT(x)    __T(x)


        template<typename T>
        struct fltr {
            static T cast(const T& v) { return v; }
            static bool isspace(char) { return false; }
            static bool isalpha(char) { return false; }
            static bool isdigit(char) { return false; }
            static bool ispunct(char) { return false; }
            static bool iscntrl(char) { return false; }
        };

        template<>
        struct fltr<std::wstring> {
            static std::wstring cast(const std::string& v) { return lins::str_util::from_utf8(v); }
            static std::wstring cast(const std::wstring& v) { return v; }
            static std::string utf8(std::wstring const &v) { return str_util::to_utf8(v); }
            static std::wstring wide(std::wstring const &v) { return v; }
            static bool isspace(std::wstring::value_type sym) { return std::iswspace(sym); }
            static bool isalpha(std::wstring::value_type sym) { bool res{static_cast<bool>(std::iswalpha(sym))}; return res; }
            static bool isdigit(std::wstring::value_type sym) { return std::iswdigit(sym); }
            static bool ispunct(std::wstring::value_type sym) { return std::iswpunct(sym); }
            static bool iscntrl(std::wstring::value_type sym) { return std::iswcntrl(sym); }
        };

        template<>
        struct fltr<std::string> {
            static std::string cast(const std::wstring& v) { return lins::str_util::to_utf8(v); }
            static std::string cast(const std::string& v) { return v; }
            static std::string utf8(std::string const &v) { return v; }
            static std::wstring wide(std::string const &v) { return str_util::from_utf8(v); }
            static bool isspace(std::string::value_type sym) { return std::isspace(sym); }
            static bool isalpha(std::string::value_type sym) { return std::isalpha(sym); }
            static bool isdigit(std::string::value_type sym) { return std::isdigit(sym); }
            static bool ispunct(std::string::value_type sym) { return std::ispunct(sym); }
            static bool iscntrl(std::string::value_type sym) { return std::iscntrl(sym); }
        };

        static std::string vec_to_str(std::vector<std::uint8_t> const &v) {
            return {v.begin(), v.end()};
        }

        static std::vector<std::uint8_t> str_to_vec(std::string const &v) {
            return {v.begin(), v.end()};
        }

        static std::string perf_letter(long double prcnt) {
            int pd{(int)std::round(prcnt * 8)};
            unsigned char buff[16];
            if(pd > 0 && pd <= 8) {
                lins::str_util::ucs_to_utf8(0x2580 + pd, buff);
            } else if(pd == 0) {
                lins::str_util::ucs_to_utf8(0x1F79D, buff);
            } else {
                buff[0] = 0;
            }
            return std::string{(char *)buff};
        }

        class char_classifier {
        public:
            enum class char_class {
                digit,
                alpha_lower,
                alpha_upper,
                blank,
                cntrl,
                space,
                punct,
                graph,
                latin_1_supplement,
                latin_extended_a,
                latin_extended_b,
                ipa_extensions,
                spacing_modifier_letters,
                combining_diacritical_marks,
                greek,
                cyrillic,
                armenian,
                hebrew,
                arabic,
                syriac,
                thaana,
                devanagari,
                bengali,
                gurmukhi,
                gujarati,
                oriya,
                tamil,
                telugu,
                kannada,
                malayalam,
                sinhala,
                thai,
                lao,
                tibetan,
                myanmar,
                georgian,
                hangul_jamo,
                ethiopic,
                cherokee,
                unified_canadian_aboriginal_syllabics,
                ogham,
                runic,
                khmer,
                mongolian,
                latin_extended_additional,
                greek_extended,
                general_punctuation,
                superscripts_and_subscripts,
                currency_symbols,
                combining_marks_for_symbols,
                letterlike_symbols,
                number_forms,
                arrows,
                mathematical_operators,
                miscellaneous_technical,
                control_pictures,
                optical_character_recognition,
                enclosed_alphanumerics,
                box_drawing,
                block_elements,
                geometric_shapes,
                miscellaneous_symbols,
                dingbats,
                braille_patterns,
                cjk_radicals_supplement,
                kangxi_radicals,
                ideographic_description_characters,
                cjk_symbols_and_punctuation,
                hiragana,
                katakana,
                bopomofo,
                hangul_compatibility_jamo,
                kanbun,
                bopomofo_extended,
                enclosed_cjk_letters_and_months,
                cjk_compatibility,
                cjk_unified_ideographs_extension_a,
                cjk_unified_ideographs,
                yi_syllables,
                yi_radicals,
                hangul_syllables,
                high_surrogates,
                high_private_use_surrogates,
                low_surrogates,
                private_use,
                cjk_compatibility_ideographs,
                alphabetic_presentation_forms,
                arabic_presentation_forms_a,
                combining_half_marks,
                cjk_compatibility_forms,
                small_form_variants,
                arabic_presentation_forms_b,
                specials,
                halfwidth_and_fullwidth_forms,
                other
            };

            char_classifier() {
                fill_charset_sizes();
            }

            static std::string char_class_name(char_class cc) {
                if((std::size_t)cc >= 0 && (std::size_t)cc < char_class_names.size()) {
                    return std::string{char_class_names[(std::size_t)cc]};
                }
                return {};
            }

            static char_class class_of_char(wchar_t c) {
                auto r{national_ranges(c)};
                if(r == char_class::other) {
                    if(std::iswalpha(c) && c < 128) {
                        r = std::iswlower(c) ? char_class::alpha_lower : char_class::alpha_upper;
                    } else if(std::iswdigit(c)) {
                        r = char_class::digit;
                    } else if(std::iswblank(c)) {
                        r = char_class::blank;
                    } else if(std::iswcntrl(c)) {
                        r = char_class::cntrl;
                    } else if(std::iswpunct(c)) {
                        r = char_class::punct;
                    } else if(std::iswgraph(c)) {
                        r = char_class::graph;
                    } else if(std::iswspace(c)) {
                        r = char_class::space;
                    }
                }
                return r;
            }

            static bool valid_class_of_char(wchar_t c) {
                auto cc{class_of_char(c)};
                return (int)cc >= 0 && (int)cc <= (int)char_class::other;
            }

            static bool valid_class(char_class cc) {
                return (int)cc >= 0 && (int)cc <= (int)char_class::other;
            }

            std::size_t class_cardinality(char_class cc) const {
                auto it{charsets_.find(cc)};
                if(it != charsets_.end()) {
                    return it->second;
                }
                return 0;
            }

            std::size_t class_cardinality(wchar_t c) const {
                return class_cardinality(class_of_char(c));
            }

        private:
            static inline std::array<std::string_view, 95> const char_class_names {
                "digit",
                "alpha_lower",
                "alpha_upper",
                "blank",
                "cntrl",
                "space",
                "punct",
                "graph",
                "latin_1_supplement",
                "latin_extended_a",
                "latin_extended_b",
                "ipa_extensions",
                "spacing_modifier_letters",
                "combining_diacritical_marks",
                "greek",
                "cyrillic",
                "armenian",
                "hebrew",
                "arabic",
                "syriac",
                "thaana",
                "devanagari",
                "bengali",
                "gurmukhi",
                "gujarati",
                "oriya",
                "tamil",
                "telugu",
                "kannada",
                "malayalam",
                "sinhala",
                "thai",
                "lao",
                "tibetan",
                "myanmar",
                "georgian",
                "hangul_jamo",
                "ethiopic",
                "cherokee",
                "unified_canadian_aboriginal_syllabics",
                "ogham",
                "runic",
                "khmer",
                "mongolian",
                "latin_extended_additional",
                "greek_extended",
                "general_punctuation",
                "superscripts_and_subscripts",
                "currency_symbols",
                "combining_marks_for_symbols",
                "letterlike_symbols",
                "number_forms",
                "arrows",
                "mathematical_operators",
                "miscellaneous_technical",
                "control_pictures",
                "optical_character_recognition",
                "enclosed_alphanumerics",
                "box_drawing",
                "block_elements",
                "geometric_shapes",
                "miscellaneous_symbols",
                "dingbats",
                "braille_patterns",
                "cjk_radicals_supplement",
                "kangxi_radicals",
                "ideographic_description_characters",
                "cjk_symbols_and_punctuation",
                "hiragana",
                "katakana",
                "bopomofo",
                "hangul_compatibility_jamo",
                "kanbun",
                "bopomofo_extended",
                "enclosed_cjk_letters_and_months",
                "cjk_compatibility",
                "cjk_unified_ideographs_extension_a",
                "cjk_unified_ideographs",
                "yi_syllables",
                "yi_radicals",
                "hangul_syllables",
                "high_surrogates",
                "high_private_use_surrogates",
                "low_surrogates",
                "private_use",
                "cjk_compatibility_ideographs",
                "alphabetic_presentation_forms",
                "arabic_presentation_forms_a",
                "combining_half_marks",
                "cjk_compatibility_forms",
                "small_form_variants",
                "arabic_presentation_forms_b",
                "specials",
                "halfwidth_and_fullwidth_forms",
                "other"
            };

        private:
            std::map<char_class, int> charsets_{};

            static char_class national_ranges(wchar_t c) {
                if(c >= 0x0080 && c <= 0x00ff) { return char_class::latin_1_supplement; } else
                if(c >= 0x0100 && c <= 0x017f) { return char_class::latin_extended_a; } else
                if(c >= 0x0180 && c <= 0x024f) { return char_class::latin_extended_b; } else
                if(c >= 0x0250 && c <= 0x02af) { return char_class::ipa_extensions; } else
                if(c >= 0x02b0 && c <= 0x02ff) { return char_class::spacing_modifier_letters; } else
                if(c >= 0x0300 && c <= 0x036f) { return char_class::combining_diacritical_marks; } else
                if(c >= 0x0370 && c <= 0x03ff) { return char_class::greek; } else
                if(c >= 0x0400 && c <= 0x04ff) { return char_class::cyrillic; } else
                if(c >= 0x0530 && c <= 0x058f) { return char_class::armenian; } else
                if(c >= 0x0590 && c <= 0x05ff) { return char_class::hebrew; } else
                if(c >= 0x0600 && c <= 0x06ff) { return char_class::arabic; } else
                if(c >= 0x0700 && c <= 0x074f) { return char_class::syriac; } else
                if(c >= 0x0780 && c <= 0x07bf) { return char_class::thaana; } else
                if(c >= 0x0900 && c <= 0x097f) { return char_class::devanagari; } else
                if(c >= 0x0980 && c <= 0x09ff) { return char_class::bengali; } else
                if(c >= 0x0a00 && c <= 0x0a7f) { return char_class::gurmukhi; } else
                if(c >= 0x0a80 && c <= 0x0aff) { return char_class::gujarati; } else
                if(c >= 0x0b00 && c <= 0x0b7f) { return char_class::oriya; } else
                if(c >= 0x0b80 && c <= 0x0bff) { return char_class::tamil; } else
                if(c >= 0x0c00 && c <= 0x0c7f) { return char_class::telugu; } else
                if(c >= 0x0c80 && c <= 0x0cff) { return char_class::kannada; } else
                if(c >= 0x0d00 && c <= 0x0d7f) { return char_class::malayalam; } else
                if(c >= 0x0d80 && c <= 0x0dff) { return char_class::sinhala; } else
                if(c >= 0x0e00 && c <= 0x0e7f) { return char_class::thai; } else
                if(c >= 0x0e80 && c <= 0x0eff) { return char_class::lao; } else
                if(c >= 0x0f00 && c <= 0x0fff) { return char_class::tibetan; } else
                if(c >= 0x1000 && c <= 0x109f) { return char_class::myanmar; } else
                if(c >= 0x10a0 && c <= 0x10ff) { return char_class::georgian; } else
                if(c >= 0x1100 && c <= 0x11ff) { return char_class::hangul_jamo; } else
                if(c >= 0x1200 && c <= 0x137f) { return char_class::ethiopic; } else
                if(c >= 0x13a0 && c <= 0x13ff) { return char_class::cherokee; } else
                if(c >= 0x1400 && c <= 0x167f) { return char_class::unified_canadian_aboriginal_syllabics; } else
                if(c >= 0x1680 && c <= 0x169f) { return char_class::ogham; } else
                if(c >= 0x16a0 && c <= 0x16ff) { return char_class::runic; } else
                if(c >= 0x1780 && c <= 0x17ff) { return char_class::khmer; } else
                if(c >= 0x1800 && c <= 0x18af) { return char_class::mongolian; } else
                if(c >= 0x1e00 && c <= 0x1eff) { return char_class::latin_extended_additional; } else
                if(c >= 0x1f00 && c <= 0x1fff) { return char_class::greek_extended; } else
                if(c >= 0x2000 && c <= 0x206f) { return char_class::general_punctuation; } else
                if(c >= 0x2070 && c <= 0x209f) { return char_class::superscripts_and_subscripts; } else
                if(c >= 0x20a0 && c <= 0x20cf) { return char_class::currency_symbols; } else
                if(c >= 0x20d0 && c <= 0x20ff) { return char_class::combining_marks_for_symbols; } else
                if(c >= 0x2100 && c <= 0x214f) { return char_class::letterlike_symbols; } else
                if(c >= 0x2150 && c <= 0x218f) { return char_class::number_forms; } else
                if(c >= 0x2190 && c <= 0x21ff) { return char_class::arrows; } else
                if(c >= 0x2200 && c <= 0x22ff) { return char_class::mathematical_operators; } else
                if(c >= 0x2300 && c <= 0x23ff) { return char_class::miscellaneous_technical; } else
                if(c >= 0x2400 && c <= 0x243f) { return char_class::control_pictures; } else
                if(c >= 0x2440 && c <= 0x245f) { return char_class::optical_character_recognition; } else
                if(c >= 0x2460 && c <= 0x24ff) { return char_class::enclosed_alphanumerics; } else
                if(c >= 0x2500 && c <= 0x257f) { return char_class::box_drawing; } else
                if(c >= 0x2580 && c <= 0x259f) { return char_class::block_elements; } else
                if(c >= 0x25a0 && c <= 0x25ff) { return char_class::geometric_shapes; } else
                if(c >= 0x2600 && c <= 0x26ff) { return char_class::miscellaneous_symbols; } else
                if(c >= 0x2700 && c <= 0x27bf) { return char_class::dingbats; } else
                if(c >= 0x2800 && c <= 0x28ff) { return char_class::braille_patterns; } else
                if(c >= 0x2e80 && c <= 0x2eff) { return char_class::cjk_radicals_supplement; } else
                if(c >= 0x2f00 && c <= 0x2fdf) { return char_class::kangxi_radicals; } else
                if(c >= 0x2ff0 && c <= 0x2fff) { return char_class::ideographic_description_characters; } else
                if(c >= 0x3000 && c <= 0x303f) { return char_class::cjk_symbols_and_punctuation; } else
                if(c >= 0x3040 && c <= 0x309f) { return char_class::hiragana; } else
                if(c >= 0x30a0 && c <= 0x30ff) { return char_class::katakana; } else
                if(c >= 0x3100 && c <= 0x312f) { return char_class::bopomofo; } else
                if(c >= 0x3130 && c <= 0x318f) { return char_class::hangul_compatibility_jamo; } else
                if(c >= 0x3190 && c <= 0x319f) { return char_class::kanbun; } else
                if(c >= 0x31a0 && c <= 0x31bf) { return char_class::bopomofo_extended; } else
                if(c >= 0x3200 && c <= 0x32ff) { return char_class::enclosed_cjk_letters_and_months; } else
                if(c >= 0x3300 && c <= 0x33ff) { return char_class::cjk_compatibility; } else
                if(c >= 0x3400 && c <= 0x4db5) { return char_class::cjk_unified_ideographs_extension_a; } else
                if(c >= 0x4e00 && c <= 0x9fff) { return char_class::cjk_unified_ideographs; } else
                if(c >= 0xa000 && c <= 0xa48f) { return char_class::yi_syllables; } else
                if(c >= 0xa490 && c <= 0xa4cf) { return char_class::yi_radicals; } else
                if(c >= 0xac00 && c <= 0xd7a3) { return char_class::hangul_syllables; } else
                if(c >= 0xd800 && c <= 0xdb7f) { return char_class::high_surrogates; } else
                if(c >= 0xdb80 && c <= 0xdbff) { return char_class::high_private_use_surrogates; } else
                if(c >= 0xdc00 && c <= 0xdfff) { return char_class::low_surrogates; } else
                if(c >= 0xe000 && c <= 0xf8ff) { return char_class::private_use; } else
                if(c >= 0xf900 && c <= 0xfaff) { return char_class::cjk_compatibility_ideographs; } else
                if(c >= 0xfb00 && c <= 0xfb4f) { return char_class::alphabetic_presentation_forms; } else
                if(c >= 0xfb50 && c <= 0xfdff) { return char_class::arabic_presentation_forms_a; } else
                if(c >= 0xfe20 && c <= 0xfe2f) { return char_class::combining_half_marks; } else
                if(c >= 0xfe30 && c <= 0xfe4f) { return char_class::cjk_compatibility_forms; } else
                if(c >= 0xfe50 && c <= 0xfe6f) { return char_class::small_form_variants; } else
                if(c >= 0xfe70 && c <= 0xfefe) { return char_class::arabic_presentation_forms_b; } else
                if(c >= 0xfeff && c <= 0xfeff) { return char_class::specials; } else
                if(c >= 0xff00 && c <= 0xffef) { return char_class::halfwidth_and_fullwidth_forms; } else
                if(c >= 0xfff0 && c <= 0xfffd) { return char_class::specials; } else { return char_class::other; }
            }

            void fill_charset_sizes() {
                charsets_.clear();
                for(int i{}; i <= 1112064; ++i) {
                    ++charsets_[class_of_char(i)];
                }
            }

        };

    }
}
