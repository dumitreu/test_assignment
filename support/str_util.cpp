#include "commondefs.hpp"
#include <map>
#include <vector>
#include <sstream>
#include "str_util.hpp"

namespace lins {

    namespace str_util {

        std::string ltrim(const std::string &s) {
            std::string::size_type pos = s.find_first_not_of(" \n\r\t");
            return pos == std::string::npos ? std::string() : s.substr(s.find_first_not_of(" \n\r\t"));
        }

        std::string rtrim(const std::string &s) {
            std::string result(s);
            result.erase(result.find_last_not_of(" \n\r\t") + 1);
            return result;
        }

        std::string trim(const std::string &s) {
            return rtrim(ltrim(s));
        }

        std::wstring ltrim(const std::wstring &s) {
            std::wstring::size_type pos = s.find_first_not_of(L" \n\r\t");
            return pos == std::wstring::npos ? std::wstring() : s.substr(s.find_first_not_of(L" \n\r\t"));
        }

        std::wstring rtrim(const std::wstring &s) {
            std::wstring result(s);
            result.erase(result.find_last_not_of(L" \n\r\t") + 1);
            return result;
        }

        std::wstring trim(const std::wstring &s) {
            return rtrim(ltrim(s));
        }

        std::string prepend(const std::string &what, char with, std::size_t up_to_size) {
            std::stringstream ss{};
            std::size_t curr_size{what.size()};
            while(curr_size < up_to_size) {
                ss << with;
                ++curr_size;
            }
            ss << what;
            return ss.str();
        }

        std::uint64_t utf8_to_ucs(char const *ut8, int *increment) {
            const unsigned char *utf8 = (const unsigned char *)ut8;
            if(utf8[0] == 0) {
                if(increment) *increment = 0;
                return -1;
            }

            if(utf8[0] < 0x80) {
                if(increment) *increment = 1;
                return utf8[0] & 0xff;
            }

            if(((utf8[0] & 0xe0) == 0xc0)) {
                if(increment) *increment = 2;
                return (((std::uint64_t)utf8[0] & 0x1f) << 6) | (((std::uint64_t)utf8[1] & 0x3f));
            }

            if(((utf8[0] & 0xf0) == 0xe0)) {
                if(increment) *increment = 3;
                return (((std::uint64_t)utf8[0] & 0x0f) << 12) | (((std::uint64_t)utf8[1] & 0x3f) << 6) | (((std::uint64_t)utf8[2] & 0x3f));
            }

            if(((utf8[0] & 0xf8) == 0xf0)) {
                if(increment) *increment = 4;
                return (((std::uint64_t)utf8[0] & 0x07) << 18) | (((std::uint64_t)utf8[1] & 0x3f) << 12) | (((std::uint64_t)utf8[2] & 0x3f) << 6) | (((std::uint64_t)utf8[3] & 0x3f));
            }

            if(((utf8[0] & 0xfc) == 0xf8)) {
                if(increment) *increment = 5;
                return (((std::uint64_t)utf8[0] & 0x03) << 24) | (((std::uint64_t)utf8[1] & 0x3f) << 18) | (((std::uint64_t)utf8[2] & 0x3f) << 12) | (((std::uint64_t)utf8[3] & 0x3f) << 6) | (((std::uint64_t)utf8[4] & 0x3f));
            }

            if(((utf8[0] & 0xfe) == 0xfc)) {
                if(increment) *increment = 6;
                return (((std::uint64_t)utf8[0] & 0x01) << 30) | (((std::uint64_t)utf8[1] & 0x3f) << 24) | (((std::uint64_t)utf8[2] & 0x3f) << 18) | (((std::uint64_t)utf8[3] & 0x3f) << 12) |
                       (((std::uint64_t)utf8[4] & 0x3f) << 6) | (((std::uint64_t)utf8[5] & 0x3f));
            }

            if(increment) *increment = 1;
            return utf8[0];
        }

        int ucs_to_utf8(std::uint32_t c, unsigned char *utf8) {
            if(c <= 0x7f) {
                utf8[0] = (unsigned char) c & 0x7f;
                utf8[1] = (unsigned char) 0;
                return 1;
            } else  if(c <= 0x7ff) {
                utf8[0] = (unsigned char)((c >> 6) & 0x1f) | 0xc0;
                utf8[1] = (unsigned char)(c & 0x3f) | 0x80;
                utf8[2] = (unsigned char) 0;
                return 2;
            } else  if(c <= 0xffff) {
                utf8[0] = (unsigned char)((c >> 12) & 0xf) | 0xe0;
                utf8[1] = (unsigned char)((c >> 6) & 0x3f) | 0x80;
                utf8[2] = (unsigned char)(c & 0x3f) | 0x80;
                utf8[3] = (unsigned char) 0;
                return 3;
            } else  if(c <= 0x1fffff) {
                utf8[0] = (unsigned char)((c >> 18) & 7) | 0xf0;
                utf8[1] = (unsigned char)((c >> 12) & 0x3f) | 0x80;
                utf8[2] = (unsigned char)((c >> 6) & 0x3f) | 0x80;
                utf8[3] = (unsigned char)(c & 0x3f) | 0x80;
                utf8[4] = (unsigned char) 0;
                return 4;
            } else  if(c <= 0x3ffffff) {
                utf8[0] = (unsigned char)((c >> 24) & 3) | 0xf8;
                utf8[1] = (unsigned char)((c >> 18) & 0x3f) | 0x80;
                utf8[2] = (unsigned char)((c >> 12) & 0x3f) | 0x80;
                utf8[3] = (unsigned char)((c >> 6) & 0x3f) | 0x80;
                utf8[4] = (unsigned char)(c & 0x3f) | 0x80;
                utf8[5] = (unsigned char) 0;
                return 5;
            } else {
                utf8[0] = (unsigned char)((c >> 1) & 1) | 0xfc;
                utf8[1] = (unsigned char)((c >> 24) & 0x3f) | 0x80;
                utf8[2] = (unsigned char)((c >> 18) & 0x3f) | 0x80;
                utf8[3] = (unsigned char)((c >> 12) & 0x3f) | 0x80;
                utf8[4] = (unsigned char)((c >> 6) & 0x3f) | 0x80;
                utf8[5] = (unsigned char)(c & 0x3f) | 0x80;
                utf8[6] = (unsigned char) 0;
                return 6;
            }
        }

        std::string to_utf8(const std::wstring &str) {
            std::stringstream result{};
            for(size_t i = 0; i < str.size(); i++) {
                char buf[20];
                int s = ucs_to_utf8(str[i], (unsigned char *)buf);
                for(int j = 0; j < s; j++) {
                    result << buf[j];
                }
            }
            return result.str();
        }

        std::wstring from_utf8(const std::string &s) {
            std::wstringstream result{};
            int increment{};
            wchar_t c{};
            size_t len = s.size();
            for(const char *utf8 = s.c_str();
                len && ((c = utf8_to_ucs(utf8, &increment)) > 0);
                utf8 += increment, len -= increment
            ) {
                result << (wchar_t)c;
            }
            return result.str();
        }

        std::wstring from_utf8(const char *s) {
            std::list<std::wstring::value_type> result{};
            int increment{};
            int c{};
            for(const char *utf8 = (const char *)s;
                s && (c = utf8_to_ucs(utf8, &increment)) > 0;
                utf8 += increment
            ) {
                result .push_back(c);
            }
            return std::wstring(result.begin(), result.end());
        }

///////////////////////////////////////////////////////////////////////

        void to_utf8(const std::wstring &str, std::string &result) {
            std::stringstream ss{};
            for(size_t i = 0; i < str.size(); i++) {
                char buf[20];
                ucs_to_utf8(str[i], (unsigned char *)buf);
                ss << buf;
            }
            result = ss.str();
        }

        void from_utf8(const std::string &s, std::wstring &result) {
            int increment{};
            int c{};
            size_t len{s.size()};
            std::wstringstream ss{};
            for(const char *utf8 = s.c_str();
                len && ((c = utf8_to_ucs(utf8, &increment)) > 0);
                utf8 += increment, len -= increment
            ) {
                ss << (wchar_t)c;
            }
            result = ss.str();
        }

        void from_utf8(const char *s, std::wstring &result) {
            std::wstringstream ss{};
            int increment{};
            int c{};
            for(const char *utf8 = s;
                (c = utf8_to_ucs(utf8, &increment)) > 0;
                utf8 += increment
            ) {
                ss << (wchar_t)c;
            }
            result = ss.str();
        }





        std::int64_t atoi(const std::string &a, int radix) {
            static const int digits[256] = {
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
                throw std::runtime_error((std::string("Invalid digit: \"") + a + "\"").c_str());
            }
            if((radix < 2) || (radix > 36)) {
                throw std::runtime_error("Invalid base system radix");
            }

            bool sign_set = false;
            bool negative = false;
            size_t str_size = a.size();
            std::int64_t result = 0;
            std::int64_t pwr = 1;
            for(int i = (int)str_size - 1; i >= 0; i--, pwr *= radix) {
                if(a[i] == '-') {
                    if(sign_set) {
                        throw std::runtime_error((std::string("Invalid digit: \"") + a + "\"").c_str());
                    }
                    sign_set = true;
                    negative = true;
                    break;
                }
                if(a[i] == '+') {
                    if(sign_set) {
                        throw std::runtime_error((std::string("Invalid digit: \"") + a + "\"").c_str());
                    }
                    sign_set = true;
                    negative = false;
                    break;
                }
                int d = (int) a[i];
                if(d > 255) {
                    throw std::runtime_error((std::string("Invalid digit: \"") + a + "\"").c_str());
                }
                int dd = digits[d];
                if((dd < radix) && (dd >= 0)) {
                    result += dd * pwr;
                } else {
                    throw std::runtime_error((std::string("Invalid digit: \"") + a + "\"").c_str());
                }
            }
            return negative ? -result : result;
        }


        std::uint64_t atoui(const std::string &a, int radix) {
            static const int digits[256] = {
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
                throw std::runtime_error((std::string("Invalid digit: \"") + a + "\"").c_str());
            }
            if((radix < 2) || (radix > 36)) {
                throw std::runtime_error("Invalid base system radix");
            }

            bool sign_set = false;
            size_t str_size = a.size();
            std::uint64_t result = 0;
            std::uint64_t pwr = 1;
            for(int i = (int)str_size - 1; i >= 0; i--, pwr *= radix) {
                if(a[i] == '-') {
                    if(sign_set) {
                        throw std::runtime_error((std::string("Invalid digit: \"") + a + "\"").c_str());
                    }
                    sign_set = true;
                    break;
                }
                if(a[i] == '+') {
                    if(sign_set) {
                        throw std::runtime_error((std::string("Invalid digit: \"") + a + "\"").c_str());
                    }
                    sign_set = true;
                    break;
                }
                int d = (int) a[i];
                if(d > 255) {
                    throw std::runtime_error((std::string("Invalid digit: \"") + a + "\"").c_str());
                }
                int dd = digits[d];
                if((dd < radix) && (dd >= 0)) {
                    result += dd * pwr;
                } else {
                    throw std::runtime_error((std::string("Invalid digit: \"") + a + "\"").c_str());
                }
            }
            return result;
        }


        std::string itoa(std::int64_t value, int radix) {
            static const char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
            if((radix < 2) || (radix > 36)) {
                throw std::runtime_error("Invalid base system radix");
            }
            if(value == 0LL) {
                return "0";
            }
            bool sign = value < 0L;
            char char_stack[256];
            int stack_ptr = -1;
            for(std::int64_t val = sign ? -value : value; val != 0; val /= radix) {
                std::int64_t k = val % radix;
                char_stack[++stack_ptr] = digits[k];
            }
            if(sign) {
                char_stack[++stack_ptr] = '-';
            }

            std::string res;
            res.reserve(stack_ptr + 1);
            for(int i = stack_ptr; i >= 0; --i) {
                res.push_back(char_stack[i]);
            }

            return res;
        }

        std::string utoa(std::uint64_t value, int radix) {
            static const char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
            if((radix < 2) || (radix > 36)) {
                throw std::runtime_error("Invalid base system radix");
            }
            if(value == 0ULL) {
                return "0";
            }
            char char_stack[256];
            int stack_ptr = -1;
            for(; value != 0; value /= radix) {
                std::uint64_t k = value % radix;
                char_stack[++stack_ptr] = digits[k];
            }
            std::string res;
            res.reserve(stack_ptr + 1);
            for(int i = stack_ptr; i >= 0; --i) {
                res.push_back(char_stack[i]);
            }

            return res;
        }

        bool str_to_bool(const std::string &s) {
            try {
                return atoi(s) != 0LL;
            } catch(...) {
                if(strtolower(s) == "true" || strtolower(s) == "yes" || strtolower(s) == "1") {
                    return true;
                }
            }
            return false;
        }

        std::string data_to_hex_str(const void *data, int data_size) {
            static char hex_digits[] = {
                '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
            };
            if(data == 0 || data_size == 0) {
                return std::string{};
            }
            std::string result{};
            result.reserve(data_size * 2);
            const std::uint8_t *byte_data = reinterpret_cast<const std::uint8_t *>(data);
            for(int i = 0; i < data_size; i++) {
                std::uint8_t byte = byte_data[i];
                result.push_back(hex_digits[(byte >> 4) & 0x0f]);
                result.push_back(hex_digits[byte & 0x0f]);
            }
            return result;
        }

        std::string data_to_hex_str(const std::vector<std::uint8_t> &data) {
            return data_to_hex_str(data.data(), data.size());
        }

        std::string data_to_hex_str(const std::string &data)  {
            return data_to_hex_str(data.data(), data.size());
        }

        std::vector<std::uint8_t> hex_str_to_data(const std::string &str) {
            static constexpr int hex_do_digit[256] {
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1,
                -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
            };
            std::string::size_type ss = str.size();
            if(ss % 2) {
                throw std::runtime_error("in hex_str_to_data(): invalid hexadecimal string size");
            }
            std::vector<std::uint8_t> result(ss / 2);
            int j = 0;
            for(std::string::size_type i = 0; i < ss;) {
                int h = hex_do_digit[static_cast<unsigned int>(str[i++])];
                if(h < 0) {
                    throw std::runtime_error("in hex_str_to_data(): invalid hexadecimal string content");
                }
                int l = hex_do_digit[static_cast<unsigned int>(str[i++])];
                if(l < 0) {
                    throw std::runtime_error("in hex_str_to_data(): invalid hexadecimal string content");
                }
                result[j++] = (h << 4) | l;
            }
            return result;
        }

        static char constexpr base64_chars[64] = {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
            'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
            'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
            'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
        };

        std::string data_to_base64_str(void const *in_data, std::size_t in_len) {
            std::string ret{};
            ret.reserve(in_len * 4 / 3 + 16);
            std::size_t i{0};
            std::size_t j{0};
            std::uint8_t char_array_3[3]{};
            std::uint8_t char_array_4[4]{};
            std::uint8_t const *bytes_to_encode{(std::uint8_t const *)in_data};

            while(in_len--) {
                char_array_3[i++] = *(bytes_to_encode++);
                if(i == 3) {
                    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                    char_array_4[3] = char_array_3[2] & 0x3f;

                    for(i = 0; i < 4; i++) {
                        ret += base64_chars[char_array_4[i]];
                    }
                    i = 0;
                }
            }

            if(i) {
                for(j = i; j < 3; j++) {
                    char_array_3[j] = 0;
                }

                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for(j = 0; j < i + 1; j++) {
                    ret += base64_chars[char_array_4[j]];
                }

                while(i++ < 3) {
                    ret += '=';
                }

            }
            return ret;
        }

        std::string data_to_base64_str(std::vector<std::uint8_t> const &data) {
            return data_to_base64_str(data.data(), data.size());
        }

        static constexpr std::array<int, 256> base64_remap = {
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
            -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
            -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
            41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
        };

        static bool is_base64(unsigned char c) {
            return (c >= 65 && c <= 90) || (c >= 97 && c <= 122) || (c >= 47 && c <= 57) || c == 43;
        }

        std::vector<std::uint8_t> base64_str_to_data(std::string const &encoded_string) {
            if(encoded_string.empty()) {
                return {};
            }

            if(encoded_string.size() % 4 != 0) {
                throw std::runtime_error{"wrong base64 string size"};
            }

            std::size_t in_len{encoded_string.size()};
            std::size_t i{0};
            std::size_t j{0};
            std::size_t in_{0};
            std::uint8_t char_array_4[4]{};
            std::uint8_t char_array_3[3]{};
            std::vector<std::uint8_t> ret{};
            ret.reserve(encoded_string.size() * 3 / 4 + 16);

            while(in_len-- && (encoded_string[in_] != '=')) {
                if(!is_base64(encoded_string[in_])) {
                    if(std::iscntrl(encoded_string[in_]) || std::isspace(encoded_string[in_])) {
                        ++in_;
                        continue;
                    } else {
                        throw std::runtime_error{"invalid base64 string"};
                    }
                }
                char_array_4[i++] = encoded_string[in_]; in_++;
                if(i == 4) {
                    for(i = 0; i < 4; i++) {
                        char_array_4[i] = base64_remap[char_array_4[i]];
                    }

                    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                    ret.insert(ret.end(), char_array_3, char_array_3 + 3);

                    i = 0;
                }
            }

            if(i) {
                for(j = i; j < 4; j++) {
                    char_array_4[j] = 0;
                }

                for(j = 0; j < 4; j++) {
                    char_array_4[j] = base64_remap[char_array_4[j]];
                }

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for(j = 0; j < i - 1; j++) {
                    ret.push_back(char_array_3[j]);
                }
            }

            return ret;
        }

        bool is_valid_base64(std::string const &may_be_b64) {
            bool result{false};
            try {
                if(may_be_b64.size()) {
                    auto recovered_data{base64_str_to_data(may_be_b64)};
                    if(recovered_data.size()) {
                        result = true;
                    }
                }
            } catch (...) {
                result = false;
            }
            return result;
        }

        std::string hexdump(const std::vector<std::uint8_t> &d, std::size_t cols, const std::string &splitter, bool control_dots) {
            if(d.size()) {
                return hexdump(d.data(), d.size(), cols, splitter, control_dots);
            } else {
                return std::string{};
            }
        }

        std::string hexdump(const std::deque<std::uint8_t> &d, std::size_t cols, const std::string &splitter, bool control_dots) {
            if(d.size()) {
                return hexdump(std::vector<std::uint8_t>{d.begin(), d.end()}, cols, splitter, control_dots);
            } else {
                return std::string{};
            }
        }

        std::string hexdump(const std::string &d, std::size_t cols, const std::string &splitter, bool control_dots) {
            if(d.size()) {
                return hexdump(std::vector<std::uint8_t>{d.begin(), d.end()}, cols, splitter, control_dots);
            } else {
                return std::string{};
            }
        }

        std::string hexdump(const void *d, std::size_t s, std::size_t cols, const std::string &splitter, bool control_dots) {
            std::stringstream res;
            if(s && d && cols) {
                if(!cols) {
                    cols = 16;
                }
                const std::uint8_t *bd = (const std::uint8_t *)d;

                std::size_t k = s / cols;
                k *= cols;
                std::string max_offs_string;
                if(k == 0) {
                    max_offs_string = "000";
                } else {
                    max_offs_string = lins::str_util::utoa((k - 1), 16);
                }
                std::size_t moss = max_offs_string.size() + 1;

                for(std::size_t offs = 0; offs < s; offs += cols) {
                    if(offs) {
                        res << '\n';
                    }

                    std::string curr_offs_string = lins::str_util::utoa(offs, 16);
                    while(curr_offs_string.size() < moss) {
                        curr_offs_string = std::string("0") + curr_offs_string;
                    }
                    res << curr_offs_string << 'h';

                    res << splitter;

                    for(std::size_t i = 0; i < cols; i++) {
                        if(i != 0) {
                            res << " ";
                        }
                        if(i == cols / 2) {
                            res << " ";
                        }
                        if(offs + i < s) {
                            std::string curr_hex_byte = lins::str_util::utoa(bd[offs + i], 16);
                            while(curr_hex_byte.size() < 2) {
                                curr_hex_byte = std::string("0") + curr_hex_byte;
                            }
                            res << curr_hex_byte;
                        } else {
                            res << "  ";
                        }
                    }

                    res << splitter;

                    char u[10];
                    u[1] = 0;
                    for(std::size_t i = 0; i < cols && offs + i < s; i++) {
                        unsigned char c = (unsigned char)bd[offs + i];
                        if(control_dots) {
                            u[0] = c >= ' ' && c < 128 ? c : '.';
                        } else {
                            if(c < ' ') {
                                int last{lins::str_util::ucs_to_utf8(c + 0x2400, (unsigned char *)u)};
                                u[last] = 0;
                            } else if(c >= 128) {
                                lins::str_util::ucs_to_utf8(c, (unsigned char *)u);
                            } else {
                                u[0] = c;
                            }
                        }
                        res << std::string{u};
                        std::memset(u, 0, sizeof(u));
                    }
                }
            }
            return res.str();
        }



#define CHAR_TYPE_LETTER_UPPERCASE              1   // Lu   Letter, Uppercase
#define CHAR_TYPE_LETTER_LOWERCASE              2   // Ll   Letter, Lowercase
#define CHAR_TYPE_LETTER_TITLECASE              3   // Lt   Letter, Titlecase
#define CHAR_TYPE_MARK_NON_SPACING              4   // Mn   Mark, Non-Spacing
#define CHAR_TYPE_MARK_SPACING_COMBINING        5   // Mc   Mark, Spacing Combining
#define CHAR_TYPE_MARK_ENCLOSING                6   // Me   Mark, Enclosing
#define CHAR_TYPE_NUMBER_DECIMAL_DIGIT          7   // Nd   Number, Decimal Digit
#define CHAR_TYPE_NUMBER_LETTER                 8   // Nl   Number, Letter
#define CHAR_TYPE_NUMBER_OTHER                  9   // No   Number, Other
#define CHAR_TYPE_SEPARATOR_SPACE               10  // Zs   Separator, Space
#define CHAR_TYPE_SEPARATOR_LINE                11  // Zl   Separator, Line
#define CHAR_TYPE_SEPARATOR_PARAGRAPH           12  // Zp   Separator, Paragraph
#define CHAR_TYPE_OTHER_CONTROL                 13  // Cc   Other, Control
#define CHAR_TYPE_OTHER_FORMAT                  14  // Cf   Other, Format
#define CHAR_TYPE_OTHER_SURROGATE               15  // Cs   Other, Surrogate
#define CHAR_TYPE_OTHER_PRIVATE_USE             16  // Co   Other, Private Use
#define CHAR_TYPE_OTHER_NOT_ASSIGNED            17  // Cn   Other, Not Assigned (no characters in the file have this property)

#define CHAR_TYPE_LETTER_MODIFIER               18  // Lm   Letter, Modifier
#define CHAR_TYPE_LETTER_OTHER                  19  // Lo   Letter, Other
#define CHAR_TYPE_PUNCTUATION_CONNECTOR         20  // Pc   Punctuation, Connector
#define CHAR_TYPE_PUNCTUATION_DASH              21  // Pd   Punctuation, Dash
#define CHAR_TYPE_PUNCTUATION_OPEN              22  // Ps   Punctuation, Open
#define CHAR_TYPE_PUNCTUATION_CLOSE             23  // Pe   Punctuation, Close
#define CHAR_TYPE_PUNCTUATION_INITIAL_QUOTE     24  // Pi   Punctuation, Initial quote (may behave like Ps or Pe depending on usage)
#define CHAR_TYPE_PUNCTUATION_FINAL_QUOTE       25  // Pf   Punctuation, Final quote (may behave like Ps or Pe depending on usage)
#define CHAR_TYPE_PUNCTUATION_OTHER             26  // Po   Punctuation, Other
#define CHAR_TYPE_SYMBOL_MATH                   27  // Sm   Symbol, Math
#define CHAR_TYPE_SYMBOL_CURRENCY               28  // Sc   Symbol, Currency
#define CHAR_TYPE_SYMBOL_MODIFIER               29  // Sk   Symbol, Modifier
#define CHAR_TYPE_SYMBOL_OTHER                  30  // So   Symbol, Other


        static std::map<std::uint32_t, std::uint8_t> char_type;
        static std::map<std::uint32_t, std::uint32_t> lower_to_upper_map;
        static std::map<std::uint32_t, std::uint32_t> upper_to_lower_map;


        class library_initializer {
        public:
            library_initializer();
        };

        library_initializer::library_initializer() {
//  std::map<std::string, std::uint8_t> char_types;
//  char_types["Lu"] = CHAR_TYPE_LETTER_UPPERCASE;
//  char_types["Ll"] = CHAR_TYPE_LETTER_LOWERCASE;
//  char_types["Lt"] = CHAR_TYPE_LETTER_TITLECASE;
//  char_types["Mn"] = CHAR_TYPE_MARK_NON_SPACING;
//  char_types["Mc"] = CHAR_TYPE_MARK_SPACING_COMBINING;
//  char_types["Me"] = CHAR_TYPE_MARK_ENCLOSING;
//  char_types["Nd"] = CHAR_TYPE_NUMBER_DECIMAL_DIGIT;
//  char_types["Nl"] = CHAR_TYPE_NUMBER_LETTER;
//  char_types["No"] = CHAR_TYPE_NUMBER_OTHER;
//  char_types["Zs"] = CHAR_TYPE_SEPARATOR_SPACE;
//  char_types["Zl"] = CHAR_TYPE_SEPARATOR_LINE;
//  char_types["Zp"] = CHAR_TYPE_SEPARATOR_PARAGRAPH;
//  char_types["Cc"] = CHAR_TYPE_OTHER_CONTROL;
//  char_types["Cf"] = CHAR_TYPE_OTHER_FORMAT;
//  char_types["Cs"] = CHAR_TYPE_OTHER_SURROGATE;
//  char_types["Co"] = CHAR_TYPE_OTHER_PRIVATE_USE;
//  char_types["Cn"] = CHAR_TYPE_OTHER_NOT_ASSIGNED;

//  char_types["Lm"] = CHAR_TYPE_LETTER_MODIFIER;
//  char_types["Lo"] = CHAR_TYPE_LETTER_OTHER;
//  char_types["Pc"] = CHAR_TYPE_PUNCTUATION_CONNECTOR;
//  char_types["Pd"] = CHAR_TYPE_PUNCTUATION_DASH;
//  char_types["Ps"] = CHAR_TYPE_PUNCTUATION_OPEN;
//  char_types["Pe"] = CHAR_TYPE_PUNCTUATION_CLOSE;
//  char_types["Pi"] = CHAR_TYPE_PUNCTUATION_INITIAL_QUOTE;
//  char_types["Pf"] = CHAR_TYPE_PUNCTUATION_FINAL_QUOTE;
//  char_types["Po"] = CHAR_TYPE_PUNCTUATION_OTHER;
//  char_types["Sm"] = CHAR_TYPE_SYMBOL_MATH;
//  char_types["Sc"] = CHAR_TYPE_SYMBOL_CURRENCY;
//  char_types["Sk"] = CHAR_TYPE_SYMBOL_MODIFIER;
//  char_types["So"] = CHAR_TYPE_SYMBOL_OTHER;

//  std::string str;
//  for(size_t i = 0; unicode_data[i]; i++) {
//      str = unicode_data[i];

//      typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
//#pragma warning(push)
//#pragma warning(disable : 4482)
//      boost::char_separator<char> sep(";", "", boost::empty_token_policy::keep_empty_tokens);
//#pragma warning(pop)
//      tokenizer tokens(str, sep);
//      std::vector<std::string> row;
//      std::copy(tokens.begin(), tokens.end(), std::back_inserter<std::vector<std::string>>(row));


//      if(row.size() == 15) {
//          std::uint8_t ct;
//          try {
//              std::uint32_t cn = (std::uint32_t) atoi(row[0], 16);
//              if(char_types.find(row[2]) != char_types.end()) {
//                  ct = char_types[row[2]];
//                  char_type[cn] = ct;
//              }
//              if(ct == CHAR_TYPE_LETTER_LOWERCASE) {
//                  try {
//                      std::uint32_t uppercase_mapping = (std::uint32_t) atoi(row[12], 16);
//                      lower_to_upper_map[cn] = uppercase_mapping;
//                  } catch(std::exception e) {
//                  }
//              }
//              if(ct == CHAR_TYPE_LETTER_UPPERCASE) {
//                  try {
//                      std::uint32_t lowercase_mapping = (std::uint32_t) atoi(row[13], 16);
//                      upper_to_lower_map[cn] = lowercase_mapping;
//                  } catch(std::exception e) {
//                  }
//              }
//          } catch(std::exception e) {
//          }
//      }
//  }
        }

        library_initializer init_lib;


        int ToWUpper(int c) {
            std::map<std::uint32_t, std::uint32_t>::iterator it = lower_to_upper_map.find(c);
            return it == lower_to_upper_map.end() ? c : it->second;
        }

        int ToWLower(int c) {
            std::map<std::uint32_t, std::uint32_t>::iterator it = upper_to_lower_map.find(c);
            return it == upper_to_lower_map.end() ? c : it->second;
        }

/////////////////////////////////////////////////////////////////////////////////////////////////////

        int towupper(int c) {
            return ToWUpper(c);
        }

        int towlower(int c) {
            return ToWLower(c);
        }

        int iswalpha(int c) {
            std::map<std::uint32_t, std::uint8_t>::iterator it = char_type.find(c);
            if(it == char_type.end()) {
                return 0;
            } else {
                int cc = it->second;
                return
                    cc == CHAR_TYPE_LETTER_UPPERCASE ||
                    cc == CHAR_TYPE_LETTER_LOWERCASE ||
                    cc == CHAR_TYPE_LETTER_TITLECASE ||
                    cc == CHAR_TYPE_LETTER_OTHER
                    ;
            }
        }

        int iswdigit(int c) {
            std::map<std::uint32_t, std::uint8_t>::iterator it = char_type.find(c);
            if(it == char_type.end()) {
                return 0;
            } else {
                int cc = it->second;
                return
                    cc == CHAR_TYPE_NUMBER_DECIMAL_DIGIT ||
                    cc == CHAR_TYPE_NUMBER_LETTER ||
                    cc == CHAR_TYPE_NUMBER_OTHER ||
                    cc == CHAR_TYPE_NUMBER_LETTER;
            }
        }

        int iswalnum(int c) {
            std::map<std::uint32_t, std::uint8_t>::iterator it = char_type.find(c);
            if(it == char_type.end()) {
                return 0;
            } else {
                int cc = it->second;
                return
                    cc == CHAR_TYPE_NUMBER_DECIMAL_DIGIT ||
                    cc == CHAR_TYPE_NUMBER_LETTER ||
                    cc == CHAR_TYPE_NUMBER_OTHER ||
                    cc == CHAR_TYPE_NUMBER_LETTER ||
                    cc == CHAR_TYPE_LETTER_UPPERCASE ||
                    cc == CHAR_TYPE_LETTER_LOWERCASE ||
                    cc == CHAR_TYPE_LETTER_TITLECASE ||
                    cc == CHAR_TYPE_LETTER_OTHER;
            }
        }

        int iswlower(int c) {
            std::map<std::uint32_t, std::uint32_t>::iterator it = lower_to_upper_map.find(c);
            return it != lower_to_upper_map.end();
        }

        int iswupper(int c) {
            std::map<std::uint32_t, std::uint32_t>::iterator it = upper_to_lower_map.find(c);
            return it != upper_to_lower_map.end();
        }

        int iswprint(int c) {
            std::map<std::uint32_t, std::uint8_t>::iterator it = char_type.find(c);
            if(it == char_type.end()) {
                return 0;
            } else {
                int cc = it->second;
                return
                    cc == CHAR_TYPE_NUMBER_DECIMAL_DIGIT ||
                    cc == CHAR_TYPE_NUMBER_LETTER ||
                    cc == CHAR_TYPE_NUMBER_OTHER ||
                    cc == CHAR_TYPE_NUMBER_LETTER ||
                    cc == CHAR_TYPE_LETTER_UPPERCASE ||
                    cc == CHAR_TYPE_LETTER_LOWERCASE ||
                    cc == CHAR_TYPE_LETTER_TITLECASE ||
                    cc == CHAR_TYPE_LETTER_OTHER;
            }
        }

        int iswgraph(int /*c*/) {
            throw std::runtime_error("int iswgraph(int c): function not implemented yet.");
            return 0;
        }

        int iswascii_(int c) {
            //return ::iswascii(c);
            return c < 128;
        }

        int iswcntrl(int c) {
            std::map<std::uint32_t, std::uint8_t>::iterator it = char_type.find(c);
            if(it == char_type.end()) {
                return 0;
            } else {
                int cc = it->second;
                return cc == CHAR_TYPE_OTHER_CONTROL;
            }
        }

//int iswpunct(int c) {
//  return ::iswpunct(c);
//}

//int iswspace(int c) {
//  return ::iswspace(c);
//}

        std::wstring wstrtolower(const std::wstring &str) {
            std::wstring res;
            res.resize(str.size());
            for(size_t i = 0; i < str.size(); i++) {
                res[i] = towlower(str[i]);
            }
            return res;
        }

        std::wstring wstrtoupper(const std::wstring &str) {
            std::wstring res;
            res.resize(str.size());
            for(size_t i = 0; i < str.size(); i++) {
                res[i] = towupper(str[i]);
            }
            return res;
        }

/////////////////////////////////////////////////////////////////////////////////////////////////////

        int toupper(int c) {
            return ::toupper(c);
        }

        int tolower(int c) {
            return ::tolower(c);
        }

        int isalpha(int c) {
            return ::isalpha(c);
        }

        int isdigit(int c) {
            return ::isdigit(c);
        }

        int isalnum(int c) {
            return ::isalnum(c);
        }

        int islower(int c) {
            return ::islower(c);
        }

        int isupper(int c) {
            return ::isupper(c);
        }

        int isprint(int c) {
            return ::isprint(c);
        }

        int isgraph(int c) {
            return ::isgraph(c);
        }

        int isascii_(int c) {
            return isascii(c);
        }

        int iscntrl(int c) {
            return ::iscntrl(c);
        }

        int ispunct(int c) {
            return ::ispunct(c);
        }

        int isspace(int c) {
            return ::isspace(c);
        }

        std::string strtolower(const std::string &str) {
            std::string res;
            res.resize(str.size());
            for(size_t i = 0; i < str.size(); i++) {
                res[i] = ::tolower(str[i]);
            }
            return res;
        }

        std::string strtoupper(const std::string &str) {
            std::string res;
            res.resize(str.size());
            for(size_t i = 0; i < str.size(); i++) {
                res[i] = ::toupper(str[i]);
            }
            return res;
        }

        std::string leave_only_mentioned(const std::string &a, const std::string &chars) {
            std::string result;
            std::set<char> allowed_set;
            for(char c: chars) {
                allowed_set.insert(c);
            }
            result.reserve(a.size());
            for(char c: a) {
                if(allowed_set.find(c) != allowed_set.cend()) {
                    result += c;
                }
            }
            return result;
        }

        bool check_only_mentioned(const std::string &a, const std::string &chars) {
            std::set<char> allowed_set;
            for(char c: chars) {
                allowed_set.insert(c);
            }
            for(char c: a) {
                if(allowed_set.find(c) != allowed_set.cend()) {
                    return false;
                }
            }
            return true;
        }

    }

}
