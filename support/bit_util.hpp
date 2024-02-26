#pragma once

#include "commondefs.hpp"
#include "sys_util.hpp"

namespace lins {

    namespace bit_util {

        std::uint64_t max_value(int bits);
        std::uint64_t analog_to_digital(double value, std::uint8_t bits);
        double digital_to_analog(std::uint64_t value, std::uint8_t bits);
        static void inplace_swap(void *data, std::size_t count) {
            if(data && count > 1) {
                std::size_t const count_half{count / 2};
                for(std::size_t i{0}; i < count_half; ++i) {
                    std::swap(*(((std::uint8_t *)data) + i), *(((std::uint8_t *)data) + count - 1 - i));
                }
            }
        }
        int bits_required(std::uint64_t v);
        int bytes_required(std::uint64_t v);
        std::uint64_t lo_mask_for_value(std::uint64_t);
        std::uint64_t lo_mask_for_bits_count(int);
        std::uint8_t low_4_bits(std::uint8_t r);
        std::uint8_t high_4_bits(std::uint8_t r);

        static inline std::uint8_t byte_flip(std::uint8_t b) {
            b = (b & 0xf0) >> 4 | (b & 0x0f) << 4;
            b = (b & 0xcc) >> 2 | (b & 0x33) << 2;
            b = (b & 0xaa) >> 1 | (b & 0x55) << 1;
            return b;
        }

        static inline std::uint8_t byte_flip_2(std::uint8_t n) {
            constexpr std::uint64_t ta{17848844570815808640ULL};
            return (((ta >> ((n & 0xf) << 2)) & 0xf) << 4) | ((ta >> (((n >> 4) & 0xf) << 2)) & 0xf);
        }

        static inline std::uint8_t byte_flip_3(std::uint8_t n) {
            static constexpr std::uint8_t map[256]{
                0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
                0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8, 0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
                0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4, 0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
                0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec, 0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
                0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2, 0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
                0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea, 0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
                0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6, 0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
                0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee, 0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
                0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1, 0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
                0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9, 0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
                0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5, 0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
                0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed, 0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
                0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3, 0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
                0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb, 0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
                0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7, 0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
                0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef, 0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
            };
            return map[n];
        }

        static std::size_t num_of_set_bits(std::uint8_t byte) {
            std::uint8_t constexpr lt[16]{0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
            return lt[(byte >> 4) & 0xf] + lt[byte & 0xf];
        }





        template<typename T>
        struct byteswap {
            byteswap(T v): val{v} { inplace_swap(&val, sizeof(T)); }
            T val;
        };



        template<typename T> struct rol;
        template<> struct rol<std::uint8_t> { std::uint8_t val; rol(std::uint8_t v, std::size_t n): val{(std::uint8_t)((v << n) | (v >> (8 - n)))} {} };
        template<> struct rol<std::int8_t> { std::int8_t val; rol(std::int8_t v, std::size_t n): val{(std::int8_t)(((std::uint8_t)v << n) | (((std::uint8_t)v) >> (8 - n)))} {} };
        template<> struct rol<std::uint16_t> { std::uint16_t val; rol(std::uint16_t v, std::size_t n): val{(std::uint16_t)(((std::uint16_t)v << n) | ((std::uint16_t)v >> (16 - n)))} {} };
        template<> struct rol<std::int16_t> { std::int16_t val; rol(std::int16_t v, std::size_t n): val{(std::int16_t)(((std::uint16_t)v << n) | (((std::uint16_t)v) >> (16 - n)))} {} };
        template<> struct rol<std::uint32_t> { std::uint32_t val; rol(std::uint32_t v, std::size_t n): val{(v << n) | (v >> (32 - n))} {} };
        template<> struct rol<std::int32_t> { std::int32_t val; rol(std::int32_t v, std::size_t n): val{(std::int32_t)(((std::uint32_t)v << n) | (((std::uint32_t)v) >> (32 - n)))} {} };
        template<> struct rol<std::uint64_t> { std::uint64_t val; rol(std::uint64_t v, std::size_t n): val{(v << n) | (v >> (64 - n))} {} };
        template<> struct rol<std::int64_t> { std::int64_t val; rol(std::int64_t v, std::size_t n): val{(std::int64_t)(((std::uint64_t)v << n) | (((std::uint64_t)v) >> (64 - n)))} {} };


        template<typename T> struct ror;
        template<> struct ror<std::uint8_t> { std::uint8_t val; ror(std::uint8_t v, std::size_t n): val{(std::uint8_t)((v >> n) | (v << (8 - n)))} {} };
        template<> struct ror<std::int8_t> { std::int8_t val; ror(std::int8_t v, std::size_t n): val{(std::int8_t)((((std::uint8_t)v) >> n) | (((std::uint8_t)v) << (8 - n)))} {} };
        template<> struct ror<std::uint16_t> { std::uint16_t val; ror(std::uint16_t v, std::size_t n): val{(std::uint16_t)((v >> n) | (v << (16 - n)))} {} };
        template<> struct ror<std::int16_t> { std::int16_t val; ror(std::int16_t v, std::size_t n): val{(std::int16_t)((((std::uint16_t)v) >> n) | (((std::uint16_t)v) << (16 - n)))} {} };
        template<> struct ror<std::uint32_t> { std::uint32_t val; ror(std::uint32_t v, std::size_t n): val{((v) >> n) | ((v) << (32 - n))} {} };
        template<> struct ror<std::int32_t> { std::int32_t val; ror(std::int32_t v, std::size_t n): val{(std::int32_t)((((std::uint32_t)v) >> n) | (((std::uint32_t)v) << (32 - n)))} {} };
        template<> struct ror<std::uint64_t> { std::uint64_t val; ror(std::uint64_t v, std::size_t n): val{(v >> n) | (v << (64 - n))} {} };
        template<> struct ror<std::int64_t> { std::int64_t val; ror(std::int64_t v, std::size_t n): val{(std::int64_t)((((std::uint64_t)v) >> n) | (((std::uint64_t)v) << (64 - n)))} {} };




        template<typename T>
        struct hnswap {
            T val;
            hnswap(T v): val{v} {
                if constexpr (lins::sys_util::little_endian()) {
                    inplace_swap(&val, sizeof(T));
                }
            }
        };

        template<>
        struct hnswap<std::uint16_t> {
            std::uint16_t val;
            hnswap(std::uint16_t v): val{v} {
                if constexpr (lins::sys_util::little_endian()) {
                    std::uint8_t *val_ptr{(std::uint8_t *)&val};
                    std::swap(val_ptr[0], val_ptr[1]);
                }
            }
        };

        template<>
        struct hnswap<std::int16_t> {
            std::int16_t val;
            hnswap(std::int16_t v): val{v} {
                if constexpr (lins::sys_util::little_endian()) {
                    std::uint8_t *val_ptr{(std::uint8_t *)&val};
                    std::swap(val_ptr[0], val_ptr[1]);
                }
            }
        };

        template<>
        struct hnswap<std::uint32_t> {
            std::uint32_t val;
            hnswap(std::uint32_t v): val{v} {
                if constexpr (lins::sys_util::little_endian()) {
                    std::uint8_t *val_ptr{(std::uint8_t *)&val};
                    std::swap(val_ptr[0], val_ptr[3]);
                    std::swap(val_ptr[1], val_ptr[2]);
                }
            }
        };

        template<>
        struct hnswap<std::int32_t> {
            std::int32_t val;
            hnswap(std::int32_t v): val{v} {
                if constexpr (lins::sys_util::little_endian()) {
                    std::uint8_t *val_ptr{(std::uint8_t *)&val};
                    std::swap(val_ptr[0], val_ptr[3]);
                    std::swap(val_ptr[1], val_ptr[2]);
                }
            }
        };

        template<>
        struct hnswap<std::uint64_t> {
            std::uint64_t val;
            hnswap(std::uint64_t v): val{v} {
                if constexpr (lins::sys_util::little_endian()) {
                    std::uint8_t *val_ptr{(std::uint8_t *)&val};
                    std::swap(val_ptr[0], val_ptr[7]);
                    std::swap(val_ptr[1], val_ptr[6]);
                    std::swap(val_ptr[2], val_ptr[5]);
                    std::swap(val_ptr[3], val_ptr[4]);
                }
            }
        };

        template<>
        struct hnswap<std::int64_t> {
            std::int64_t val;
            hnswap(std::int64_t v): val{v} {
                if constexpr (lins::sys_util::little_endian()) {
                    std::uint8_t *val_ptr{(std::uint8_t *)&val};
                    std::swap(val_ptr[0], val_ptr[7]);
                    std::swap(val_ptr[1], val_ptr[6]);
                    std::swap(val_ptr[2], val_ptr[5]);
                    std::swap(val_ptr[3], val_ptr[4]);
                }
            }
        };




        template<typename T>
        struct swap_on_le {
            T val;
            swap_on_le(T v): val{v} {
                if constexpr (lins::sys_util::little_endian()) {
                    inplace_swap(&val, sizeof(T));
                }
            }
        };

        template<>
        struct swap_on_le<std::uint16_t> {
            std::uint16_t val;
            swap_on_le(std::uint16_t v): val{v} {
                if constexpr (lins::sys_util::little_endian()) {
                    std::uint8_t *val_ptr{(std::uint8_t *)&val};
                    std::swap(val_ptr[0], val_ptr[1]);
                }
            }
        };

        template<>
        struct swap_on_le<std::int16_t> {
            std::int16_t val;
            swap_on_le(std::int16_t v): val{v} {
                if constexpr (lins::sys_util::little_endian()) {
                    std::uint8_t *val_ptr{(std::uint8_t *)&val};
                    std::swap(val_ptr[0], val_ptr[1]);
                }
            }
        };

        template<>
        struct swap_on_le<std::uint32_t> {
            std::uint32_t val;
            swap_on_le(std::uint32_t v): val{v} {
                if constexpr (lins::sys_util::little_endian()) {
                    std::uint8_t *val_ptr{(std::uint8_t *)&val};
                    std::swap(val_ptr[0], val_ptr[3]);
                    std::swap(val_ptr[1], val_ptr[2]);
                }
            }
        };

        template<>
        struct swap_on_le<std::int32_t> {
            std::int32_t val;
            swap_on_le(std::int32_t v): val{v} {
                if constexpr (lins::sys_util::little_endian()) {
                    std::uint8_t *val_ptr{(std::uint8_t *)&val};
                    std::swap(val_ptr[0], val_ptr[3]);
                    std::swap(val_ptr[1], val_ptr[2]);
                }
            }
        };

        template<>
        struct swap_on_le<std::uint64_t> {
            std::uint64_t val;
            swap_on_le(std::uint64_t v): val{v} {
                if constexpr (lins::sys_util::little_endian()) {
                    std::uint8_t *val_ptr{(std::uint8_t *)&val};
                    std::swap(val_ptr[0], val_ptr[7]);
                    std::swap(val_ptr[1], val_ptr[6]);
                    std::swap(val_ptr[2], val_ptr[5]);
                    std::swap(val_ptr[3], val_ptr[4]);
                }
            }
        };

        template<>
        struct swap_on_le<std::int64_t> {
            std::int64_t val;
            swap_on_le(std::int64_t v): val{v} {
                if constexpr (lins::sys_util::little_endian()) {
                    std::uint8_t *val_ptr{(std::uint8_t *)&val};
                    std::swap(val_ptr[0], val_ptr[7]);
                    std::swap(val_ptr[1], val_ptr[6]);
                    std::swap(val_ptr[2], val_ptr[5]);
                    std::swap(val_ptr[3], val_ptr[4]);
                }
            }
        };




        template<typename T>
        struct swap_on_be {
            T val;
            swap_on_be(T v): val{v} {
                if constexpr (lins::sys_util::big_endian()) {
                    inplace_swap(&val, sizeof(T));
                }
            }
        };

        template<>
        struct swap_on_be<std::uint16_t> {
            std::uint16_t val;
            swap_on_be(std::uint16_t v): val{v} {
                if constexpr (lins::sys_util::big_endian()) {
                    std::uint8_t *val_ptr{(std::uint8_t *)&val};
                    std::swap(val_ptr[0], val_ptr[1]);
                }
            }
        };

        template<>
        struct swap_on_be<std::int16_t> {
            std::int16_t val;
            swap_on_be(std::int16_t v): val{v} {
                if constexpr (lins::sys_util::big_endian()) {
                    std::uint8_t *val_ptr{(std::uint8_t *)&val};
                    std::swap(val_ptr[0], val_ptr[1]);
                }
            }
        };

        template<>
        struct swap_on_be<std::uint32_t> {
            std::uint32_t val;
            swap_on_be(std::uint32_t v): val{v} {
                if constexpr (lins::sys_util::big_endian()) {
                    std::uint8_t *val_ptr{(std::uint8_t *)&val};
                    std::swap(val_ptr[0], val_ptr[3]);
                    std::swap(val_ptr[1], val_ptr[2]);
                }
            }
        };

        template<>
        struct swap_on_be<std::int32_t> {
            std::int32_t val;
            swap_on_be(std::int32_t v): val{v} {
                if constexpr (lins::sys_util::big_endian()) {
                    std::uint8_t *val_ptr{(std::uint8_t *)&val};
                    std::swap(val_ptr[0], val_ptr[3]);
                    std::swap(val_ptr[1], val_ptr[2]);
                }
            }
        };

        template<>
        struct swap_on_be<std::uint64_t> {
            std::uint64_t val;
            swap_on_be(std::uint64_t v): val{v} {
                if constexpr (lins::sys_util::big_endian()) {
                    std::uint8_t *val_ptr{(std::uint8_t *)&val};
                    std::swap(val_ptr[0], val_ptr[7]);
                    std::swap(val_ptr[1], val_ptr[6]);
                    std::swap(val_ptr[2], val_ptr[5]);
                    std::swap(val_ptr[3], val_ptr[4]);
                }
            }
        };

        template<>
        struct swap_on_be<std::int64_t> {
            std::int64_t val;
            swap_on_be(std::int64_t v): val{v} {
                if constexpr (lins::sys_util::big_endian()) {
                    std::uint8_t *val_ptr{(std::uint8_t *)&val};
                    std::swap(val_ptr[0], val_ptr[7]);
                    std::swap(val_ptr[1], val_ptr[6]);
                    std::swap(val_ptr[2], val_ptr[5]);
                    std::swap(val_ptr[3], val_ptr[4]);
                }
            }
        };

#ifdef __SIZEOF_INT128__
        template<>
        struct swap_on_be<unsigned __int128> {
            unsigned __int128 val;
            swap_on_be(unsigned __int128 v): val{v} {
                if constexpr (lins::sys_util::big_endian()) {
                    std::uint8_t *val_ptr{(std::uint8_t *)&val};
                    std::swap(val_ptr[0], val_ptr[15]);
                    std::swap(val_ptr[1], val_ptr[14]);
                    std::swap(val_ptr[2], val_ptr[13]);
                    std::swap(val_ptr[3], val_ptr[12]);
                    std::swap(val_ptr[4], val_ptr[11]);
                    std::swap(val_ptr[5], val_ptr[10]);
                    std::swap(val_ptr[6], val_ptr[9]);
                    std::swap(val_ptr[7], val_ptr[8]);
                }
            }
        };

        template<>
        struct swap_on_be<__int128> {
            __int128 val;
            swap_on_be(__int128 v): val{v} {
                if constexpr (lins::sys_util::big_endian()) {
                    std::uint8_t *val_ptr{(std::uint8_t *)&val};
                    std::swap(val_ptr[0], val_ptr[15]);
                    std::swap(val_ptr[1], val_ptr[14]);
                    std::swap(val_ptr[2], val_ptr[13]);
                    std::swap(val_ptr[3], val_ptr[12]);
                    std::swap(val_ptr[4], val_ptr[11]);
                    std::swap(val_ptr[5], val_ptr[10]);
                    std::swap(val_ptr[6], val_ptr[9]);
                    std::swap(val_ptr[7], val_ptr[8]);
                }
            }
        };
#endif

        #define ALIGN_BYTE  0
        #define ALIGN_WORD  1
        #define ALIGN_DWORD 2
        #define ALIGN_PARA  4
        #define ALIGN_LINE  6
        #define ALIGN_PAGE  12

        template<std::size_t BITS>
        std::size_t align_up(std::size_t s) {
            if(s & ((1 << BITS) - 1)) {
                return (s & ~((1 << BITS) - 1)) + (1 << BITS);
            } else {
                return s;
            }
        }

        template <typename T>
        void bsort(T *array, std::size_t length) {
            ssize_t i, j;
            for(i = length - 1; i >= 0; i--) {
                int s = 0;
                for(j = 0; j < i; j++) {
                    if(array[j + 1] < array[j]) {
                        T temp = array[j];
                        array[j] = array[j + 1];
                        array[j + 1] = temp;
                        s++;
                    }
                }
                if(s == 0) {
                    break;
                }
            }
        }

        template <typename T>
        class prim_bit_mask {
        public:
            class bit {
                friend class prim_bit_mask;
            private:
                bit(const bit &that): owner_(that.owner_), no_(that.no_) {}
                bit(prim_bit_mask &owner, unsigned int no): owner_(owner), no_(no) {}

            public:
                operator int() const {
                    return owner_.is_set(no_) ? 1 : 0;
                }

                operator bool() const {
                    return owner_.is_set(no_);
                }

                bit &operator=(bool value) {
                    if(value) {
                        owner_.set_bit(no_);
                    } else {
                        owner_.clear_bit(no_);
                    }
                    return *this;
                }

                bit &operator=(const bit &other) {
                    if(other) {
                        owner_.set_bit(no_);
                    } else {
                        owner_.clear_bit(no_);
                    }
                    return *this;
                }

                void set() {
                    owner_.set_bit(no_);
                }

                void clear() {
                    owner_.clear_bit(no_);
                }

            private:
                prim_bit_mask &owner_;
                unsigned int no_;
            };

            prim_bit_mask(T value = T()) : value_(value) {
            }

            prim_bit_mask(const prim_bit_mask &that) : value_(that.value_) {
            }

            prim_bit_mask &operator=(const prim_bit_mask &that) {
                if(&that != this) {
                    value_ = that.value_;
                }
                return *this;
            }

            bool is_set(unsigned int bit_no) const {
                if(bit_no < size()) {
                    return (value_ & (((T)1) << bit_no)) != 0;
                }
                return false;
            }

            void set_bit(unsigned int bit_no) {
                if(bit_no < size()) {
                    value_ |= ((T)1) << bit_no;
                }
            }

            void clear_bit(unsigned int bit_no) {
                if(bit_no < size()) {
                    value_ &= ~(T)(1 << bit_no);
                }
            }

            std::size_t size() const {
                return sizeof(T) * 8;
            }

            operator T() const {
                return value_;
            }

            bit operator[](unsigned int bit_no) {
                return bit(*this, bit_no);
            }

        private:
            T value_;
        };


        class bit_map {
        public:
            class value_proxy;
            class const_value_proxy;

            bit_map(): current_bits_count_{0} {}

            void set_bit(std::size_t bit_no, int value) {
#ifdef DEBUG_CHECK_BITMASK_RANGE
                check_range(bit_no);
#endif
                std::size_t byte_no = bit_no / 8;
                if(value) {
                    contents_[byte_no] = contents_[byte_no] | (1 << (bit_no % 8));
                } else {
                    contents_[byte_no] = contents_[byte_no] & (~(1 << (bit_no % 8)));
                }
            }

            int bit(std::size_t bit_no) const {
                if(bit_no >= current_bits_count_) {
                    throw std::out_of_range{"bit index out of range"};
                }
                return (contents_[bit_no / 8] >> (bit_no % 8)) & 1;
            }

            int bit(std::size_t bit_no) {
#ifdef DEBUG_CHECK_BITMASK_RANGE
                check_range(bit_no);
#endif
                return (contents_[bit_no / 8] >> (bit_no % 8)) & 1;
            }

            value_proxy operator[](std::size_t bit_no) {
                return value_proxy(*this, bit_no);
            }

            const const_value_proxy operator[](std::size_t bit_no) const {
                return const_value_proxy(*this, bit_no);
            }

            std::size_t size() const {
                return current_bits_count_;
            }

            std::size_t size_in_bytes() const {
                return contents_.size();
            }

            void resize(std::size_t new_bits_count) {
                contents_.resize(new_bits_count / 8 + (new_bits_count % 8 ? 1 : 0));
                current_bits_count_ = new_bits_count;
            }

            void reserve(std::size_t bits_count) {
                contents_.reserve(bits_count / 8 + (bits_count % 8 ? 1 : 0));
            }

            void set_all() {
                std::memset(&contents_[0], 0xff, contents_.size());
            }

            void unset_all() {
                std::memset(&contents_[0], 0, contents_.size());
            }

            std::uint8_t const *data() const {
                return (std::uint8_t const *)&contents_[0];
            }

            std::string bin_dump() const {
                std::stringstream ss;
                for(std::size_t i{}; i < size(); ++i) {
                    ss << bit(i);
                }
                return ss.str();
            }

            class value_proxy {
            friend class bit_map;
            private:
                value_proxy(bit_map &parent, std::size_t bit_no):
                    parent_{parent},
                    bit_no_{bit_no}
                {
                }

            public:
                ~value_proxy() = default;
                value_proxy(value_proxy &&that) = default;
                value_proxy(const value_proxy &that) = default;
                value_proxy &operator=(value_proxy &&that) = delete;
                value_proxy &operator=(value_proxy const &that) {
                    if(&that != this) {
                        parent_.set_bit(bit_no_, that.operator int());
                    }
                    return *this;
                }

                value_proxy &operator=(const_value_proxy const &that) {
                    parent_.set_bit(bit_no_, that.operator int());
                    return *this;
                }

                void operator=(int value) {
                    parent_.set_bit(bit_no_, value);
                }

                operator int() const {
                    return parent_.bit(bit_no_);
                }

            private:
                bit_map &parent_;
                std::size_t bit_no_;
            };

            class const_value_proxy {
            friend class bit_map;
            private:
                const_value_proxy(bit_map const &parent, std::size_t bit_no):
                    parent_(parent),
                    bit_no_(bit_no)
                {
                }

            public:
                ~const_value_proxy() = default;
                const_value_proxy(const_value_proxy &&that) = default;
                const_value_proxy(const_value_proxy const &that) = default;
                const_value_proxy &operator=(const_value_proxy &&that) = delete;
                const_value_proxy &operator=(const const_value_proxy &that) = delete;

                operator int() const {
                    return parent_.bit(bit_no_);
                }

            private:
                bit_map const &parent_;
                std::size_t bit_no_;
            };

        private:
            void check_range(std::size_t bit_index) {
                std::size_t byte_no = bit_index / 8;
                if(contents_.size() <= byte_no) {
                    contents_.resize(byte_no + 1);
                }
                if(current_bits_count_ <= bit_index) {
                    current_bits_count_ = bit_index + 1;
                }
            }

        private:
            std::vector<std::uint8_t> contents_{};
            std::size_t current_bits_count_{0};
        };


        class bit_map_inplace {
        public:
            class value_proxy;
            class const_value_proxy;

            bit_map_inplace(void *contents, std::size_t bits_count):
                contents_{(std::uint8_t *)contents},
                current_bits_count_{bits_count}
            {
            }

            void set_bit(std::size_t bit_no, int value) {
#ifdef DEBUG_CHECK_BITMASK_RANGE
                check_range(bit_no);
#endif
                std::size_t byte_no = bit_no / 8;
                if(value) {
                    contents_[byte_no] = contents_[byte_no] | (1 << (bit_no % 8));
                } else {
                    contents_[byte_no] = contents_[byte_no] & (~(1 << (bit_no % 8)));
                }
            }

            int bit(std::size_t bit_no) const {
#ifdef DEBUG_CHECK_BITMASK_RANGE
                check_range(bit_no);
#endif
                return (contents_[bit_no / 8] >> (bit_no % 8)) & 1;
            }

            value_proxy operator[](std::size_t bit_no) {
                return value_proxy(*this, bit_no);
            }

            const const_value_proxy operator[](std::size_t bit_no) const {
                return const_value_proxy(*this, bit_no);
            }

            std::size_t size() const {
                return current_bits_count_;
            }

            std::size_t size_in_bytes() const {
                return current_bits_count_ / 8 + (current_bits_count_ % 8 ? 1 : 0);
            }

            void set_data(void *contents, std::size_t bits_count) {
                contents_ = (std::uint8_t *)contents;
                current_bits_count_ = bits_count;
            }

            void set_all() {
                std::memset(data(), 0xff, size_in_bytes());
            }

            void unset_all() {
                std::memset(data(), 0, size_in_bytes());
            }

            std::uint8_t const *data() const {
                if(!contents_) {
                    throw std::runtime_error{"data is not initialized"};
                }
                return contents_;
            }

            std::uint8_t *data() {
                if(!contents_) {
                    throw std::runtime_error{"data is not initialized"};
                }
                return contents_;
            }

            std::string bin_dump() const {
                std::stringstream ss;
                for(std::size_t i{}; i < size(); ++i) {
                    ss << bit(i);
                }
                return ss.str();
            }

            class value_proxy {
            friend class bit_map_inplace;
            private:
                value_proxy(bit_map_inplace &parent, std::size_t bit_no):
                    parent_{parent},
                    bit_no_{bit_no}
                {
                }

            public:
                ~value_proxy() = default;
                value_proxy(value_proxy &&that) = default;
                value_proxy(const value_proxy &that) = default;
                value_proxy &operator=(value_proxy &&that) = delete;
                value_proxy &operator=(value_proxy const &that) {
                    if(&that != this) {
                        parent_.set_bit(bit_no_, that.operator int());
                    }
                    return *this;
                }

                value_proxy &operator=(const_value_proxy const &that) {
                    parent_.set_bit(bit_no_, that.operator int());
                    return *this;
                }

                void operator=(int value) {
                    parent_.set_bit(bit_no_, value);
                }

                operator int() const {
                    return parent_.bit(bit_no_);
                }

            private:
                bit_map_inplace &parent_;
                std::size_t bit_no_;
            };

            class const_value_proxy {
            friend class bit_map_inplace;
            private:
                const_value_proxy(bit_map_inplace const &parent, std::size_t bit_no):
                    parent_(parent),
                    bit_no_(bit_no)
                {
                }

            public:
                ~const_value_proxy() = default;
                const_value_proxy(const_value_proxy &&that) = default;
                const_value_proxy(const_value_proxy const &that) = default;
                const_value_proxy &operator=(const_value_proxy &&that) = delete;
                const_value_proxy &operator=(const const_value_proxy &that) = delete;

                operator int() const {
                    return parent_.bit(bit_no_);
                }

            private:
                bit_map_inplace const &parent_;
                std::size_t bit_no_;
            };

        private:
            void check_range(std::size_t bit_index) const {
                if(!contents_) {
                    throw std::runtime_error{"data is not initialized"};
                }
                if(bit_index >= current_bits_count_) {
                    throw std::out_of_range{"bit index out of range"};
                }
            }

        private:
            std::uint8_t *contents_{nullptr};
            std::size_t current_bits_count_{0};
        };


        class const_bit_map_inplace {
        public:
            class const_value_proxy;

            const_bit_map_inplace(void const *contents, std::size_t bits_count):
                contents_{(std::uint8_t const *)contents},
                current_bits_count_{bits_count}
            {
            }

            int bit(std::size_t bit_no) const {
#ifdef DEBUG_CHECK_BITMASK_RANGE
                check_range(bit_no);
#endif
                return (contents_[bit_no / 8] >> (bit_no % 8)) & 1;
            }

            const_value_proxy operator[](std::size_t bit_no) const {
                return const_value_proxy{*this, bit_no};
            }

            std::size_t size() const {
                return current_bits_count_;
            }

            std::size_t size_in_bytes() const {
                return current_bits_count_ / 8 + (current_bits_count_ % 8 ? 1 : 0);
            }

            void set_data(void *contents, std::size_t bits_count) {
                contents_ = (std::uint8_t *)contents;
                current_bits_count_ = bits_count;
            }

            std::uint8_t const *data() const {
                if(!contents_) {
                    throw std::runtime_error{"data is not initialized"};
                }
                return contents_;
            }

            std::string bin_dump() const {
                std::stringstream ss;
                for(std::size_t i{}; i < size(); ++i) {
                    ss << bit(i);
                }
                return ss.str();
            }

            class const_value_proxy {
                friend class const_bit_map_inplace;
            private:
                const_value_proxy(const_bit_map_inplace const &parent, std::size_t bit_no):
                    parent_{parent},
                    bit_no_{bit_no}
                {
                }

            public:
                ~const_value_proxy() = default;
                const_value_proxy(const_value_proxy &&that) = default;
                const_value_proxy(const_value_proxy const &that) = default;
                const_value_proxy &operator=(const_value_proxy &&that) = delete;
                const_value_proxy &operator=(const const_value_proxy &that) = delete;
                operator int() const { return parent_.bit(bit_no_); }

            private:
                const_bit_map_inplace const &parent_;
                std::size_t bit_no_;
            };

        private:
            void check_range(std::size_t bit_index) const {
                if(!contents_) {
                    throw std::runtime_error{"data is not initialized"};
                }
                if(bit_index >= current_bits_count_) {
                    throw std::out_of_range{"bit index out of range"};
                }
            }

        private:
            std::uint8_t const *contents_{nullptr};
            std::size_t current_bits_count_{0};
        };

    }
}
