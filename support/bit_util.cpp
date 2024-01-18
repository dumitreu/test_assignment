#include "commondefs.hpp"
#include "bit_util.hpp"

namespace lins {

    namespace bit_util {

        std::uint64_t analog_to_digital(real value, std::uint8_t bits) {
            if(value < 0.0) {
                return 0;
            }
            std::uint64_t max_value = (1 << bits) - 1;
            if(value > 1.0) {
                return max_value;
            }
            return max_value * value;
        }

        real digital_to_analog(std::uint64_t value, std::uint8_t bits) {
            if(!value) {
                return 0.0;
            }
            std::uint64_t max_v = (1 << bits) - 1;
            return ((real)(value & max_v)) / max_v;
        }

        std::uint64_t max_value(int bits) {
            return bits >= 64 ? 0xffffffffffffffffULL : (1ULL << bits) - 1ULL;
        }

        std::uint64_t lo_mask_for_value(std::uint64_t v) {
            v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v |= v >> 32;
            return v;
        }

        std::uint64_t lo_mask_for_bits_count(int cnt) {
            std::uint64_t res = 0ULL;
            if(cnt) {
                if(cnt > 64) {
                    cnt = 64;
                }
                res = lo_mask_for_value(1ULL << (cnt - 1));
            }
            return res;
        }

        int bits_required(std::uint64_t v) {
            if(v == 0ULL) {
                return 0;
            }
            v = lo_mask_for_value(v);
            int res = 0;
            int sh = 32;
            std::uint64_t tmp = 0;
            while(sh) {
                while((tmp = (v >> sh))) {
                    v = tmp;
                    res += sh;
                }
                sh >>= 1;
            }
            return res + 1;
        }

        int bytes_required(std::uint64_t v) {
            int b = bits_required(v);
            return b / 8 + (b % 8 ? 1 : 0);
        }

        std::uint8_t low_4_bits(std::uint8_t r) {
            return r & 0xf;
        }

        std::uint8_t high_4_bits(std::uint8_t r) {
            return (r >> 4) & 0xf;
        }

    }

}
