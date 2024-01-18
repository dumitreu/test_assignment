#pragma once

#include "commondefs.hpp"
#ifdef USE_VEX_BASE64
#include "base64/include/libbase64.h"
#endif
#include "str_util.hpp"

namespace lins {

    static std::string data_to_base64_str(void const *in_data, std::size_t in_len) {
#ifdef USE_VEX_BASE64
        if(in_data && in_len) {
            std::string res{};
            size_t new_len{in_len * 4 / 3};
            size_t new_len_r{new_len % 4 ? 4 - new_len % 4 : 0};
            res.resize(new_len + new_len_r);
            size_t outlen;
            base64_encode((char const *)in_data, in_len, res.data(), &outlen, 0);
            return res;
        }
        return {};
#else
        return lins::str_util::data_to_base64_str(in_data, in_len);
#endif

    }

    static std::string data_to_base64_str(std::vector<std::uint8_t> const &in_data) {
        return data_to_base64_str(in_data.data(), in_data.size());
    }

    static std::vector<std::uint8_t> base64_str_to_data(std::string const &encoded_string) {
#ifdef USE_VEX_BASE64
        std::vector<std::uint8_t> res{};
        if(!encoded_string.empty()) {
            res.resize(encoded_string.size() * 3 / 4 + 4);
            size_t outlen;
            int decres{base64_decode((char const *)encoded_string.data(), encoded_string.size(), (char *)res.data(), &outlen, 0)};
            if(decres == 1) {
                res.resize(outlen);
            } else {
                throw std::runtime_error{"invalid base64 string"};
            }
        }
        return res;
#else
        return lins::str_util::base64_str_to_data(encoded_string);
#endif
    }

}
