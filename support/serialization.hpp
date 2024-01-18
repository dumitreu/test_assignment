#pragma once

#include "commondefs.hpp"
#include "sys_util.hpp"
#include "bit_util.hpp"

// this should be 1 or 0
#define USE_MANUAL_MEMORY_COPYING 0

namespace lins {

#pragma pack(push, 1)
    namespace details_serial {

        class bad_serial_cast: public std::bad_cast {
        public:
            bad_serial_cast(const std::string &err): msg_{err} {
            }

            ~bad_serial_cast() noexcept {
            }

            const char* what() const noexcept {
                return msg_.c_str();
            }

        private:
            std::string msg_;
        };


        static std::size_t n_size(std::uint8_t first) {
            if(first < 0x80) { return 1; }
            if(((first & 0xe0) == 0xc0)) { return 2; }
            if(((first & 0xf0) == 0xe0)) { return 3; }
            if(((first & 0xf8) == 0xf0)) { return 4; }
            if(((first & 0xfc) == 0xf8)) { return 5; }
            if(((first & 0xfe) == 0xfc)) { return 6; }
            return 0;
        }

        static std::uint64_t m_to_n(const std::uint8_t *mbuf, int *increment) {
            if(mbuf[0] < 0x80) {
                if(increment) *increment = 1;
                return mbuf[0] & 0xff;
            }
            if(((mbuf[0] & 0xe0) == 0xc0)) {
                if(increment) *increment = 2;
                return (((std::uint64_t)mbuf[0] & 0x1f) << 6) |
                        (((std::uint64_t)mbuf[1] & 0x3f));
            }
            if(((mbuf[0] & 0xf0) == 0xe0)) {
                if(increment) *increment = 3;
                return (((std::uint64_t)mbuf[0] & 0x0f) << 12) |
                        (((std::uint64_t)mbuf[1] & 0x3f) << 6) |
                        (((std::uint64_t)mbuf[2] & 0x3f));
            }
            if(((mbuf[0] & 0xf8) == 0xf0)) {
                if(increment) *increment = 4;
                return (((std::uint64_t)mbuf[0] & 0x07) << 18) |
                        (((std::uint64_t)mbuf[1] & 0x3f) << 12) |
                        (((std::uint64_t)mbuf[2] & 0x3f) << 6) |
                        (((std::uint64_t)mbuf[3] & 0x3f));
            }
            if(((mbuf[0] & 0xfc) == 0xf8)) {
                if(increment) *increment = 5;
                return (((std::uint64_t)mbuf[0] & 0x03) << 24) |
                        (((std::uint64_t)mbuf[1] & 0x3f) << 18) |
                        (((std::uint64_t)mbuf[2] & 0x3f) << 12) |
                        (((std::uint64_t)mbuf[3] & 0x3f) << 6) |
                        (((std::uint64_t)mbuf[4] & 0x3f));
            }
            if(((mbuf[0] & 0xfe) == 0xfc)) {
                if(increment) *increment = 6;
                return (((std::uint64_t)mbuf[0] & 0x01) << 30) |
                        (((std::uint64_t)mbuf[1] & 0x3f) << 24) |
                        (((std::uint64_t)mbuf[2] & 0x3f) << 18) |
                        (((std::uint64_t)mbuf[3] & 0x3f) << 12) |
                       (((std::uint64_t)mbuf[4] & 0x3f) << 6) |
                        (((std::uint64_t)mbuf[5] & 0x3f));
            }
            if(increment) *increment = 0;
            return mbuf[0];
        }

        static int n_to_m(std::uint32_t c, std::uint8_t *mbuf) {
            if(c <= 0x7f) {
                mbuf[0] = (std::uint8_t) c & 0x7f;
                return 1;
            } else  if(c <= 0x7ff) {
                mbuf[0] = (std::uint8_t)((c >> 6) & 0x1f) | 0xc0;
                mbuf[1] = (std::uint8_t)(c & 0x3f) | 0x80;
                return 2;
            } else  if(c <= 0xffff) {
                mbuf[0] = (std::uint8_t)((c >> 12) & 0xf) | 0xe0;
                mbuf[1] = (std::uint8_t)((c >> 6) & 0x3f) | 0x80;
                mbuf[2] = (std::uint8_t)(c & 0x3f) | 0x80;
                return 3;
            } else  if(c <= 0x1fffff) {
                mbuf[0] = (std::uint8_t)((c >> 18) & 7) | 0xf0;
                mbuf[1] = (std::uint8_t)((c >> 12) & 0x3f) | 0x80;
                mbuf[2] = (std::uint8_t)((c >> 6) & 0x3f) | 0x80;
                mbuf[3] = (std::uint8_t)(c & 0x3f) | 0x80;
                return 4;
            } else  if(c <= 0x3ffffff) {
                mbuf[0] = (std::uint8_t)((c >> 24) & 3) | 0xf8;
                mbuf[1] = (std::uint8_t)((c >> 18) & 0x3f) | 0x80;
                mbuf[2] = (std::uint8_t)((c >> 12) & 0x3f) | 0x80;
                mbuf[3] = (std::uint8_t)((c >> 6) & 0x3f) | 0x80;
                mbuf[4] = (std::uint8_t)(c & 0x3f) | 0x80;
                return 5;
            } else {
                mbuf[0] = (std::uint8_t)((c >> 1) & 1) | 0xfc;
                mbuf[1] = (std::uint8_t)((c >> 24) & 0x3f) | 0x80;
                mbuf[2] = (std::uint8_t)((c >> 18) & 0x3f) | 0x80;
                mbuf[3] = (std::uint8_t)((c >> 12) & 0x3f) | 0x80;
                mbuf[4] = (std::uint8_t)((c >> 6) & 0x3f) | 0x80;
                mbuf[5] = (std::uint8_t)(c & 0x3f) | 0x80;
                return 6;
            }
        }

        static int unumber_to_mem(std::size_t s, void *bytes) {
            return n_to_m(s, reinterpret_cast<std::uint8_t *>(bytes));
        }

        static int n_bytes(const std::size_t num) {
            std::uint8_t mbuf[16];
            return unumber_to_mem(num, mbuf);
        }

        static int mem_to_unumber(void const *bytes, std::size_t &num) {
            int incr;
            num = m_to_n(reinterpret_cast<std::uint8_t const *>(bytes), &incr);
            return incr;
        }


        class size_surrounded_buffer final {
        private:
            std::size_t size_size() const {
                std::size_t size_val;
                return mem_to_unumber(buffer_start(), size_val);
            }

        public:
            std::size_t size() const {
                std::size_t size_val;
                mem_to_unumber(buffer_start(), size_val);
                return size_val;
            }

            void *data() {
                return buffer_start() + size_size();
            }

            void const *data() const {
                return buffer_start() + size_size();
            }

            void set_sizes(std::size_t s) {
                unumber_to_mem(s, buffer_start());
                unumber_to_mem(s, reinterpret_cast<std::uint8_t *>(data()) + s);
                lins::bit_util::inplace_swap(reinterpret_cast<std::uint8_t *>(data()) + s, size_size());
            }

            void set_data(const void *d, std::size_t s) {
                if(d && s) {
                    set_sizes(s);
#if USE_MANUAL_MEMORY_COPYING
                    // this satisfies clang in the sense of operands sizes
                    // but memcpy() is probably faster because of possibly optimized implementation
                    // so let memcpy() operates by default

                    std::uint8_t *dd{reinterpret_cast<std::uint8_t *>(data())};
                    for(std::size_t i{0}; i < s; ++i) {
                        dd[i] = reinterpret_cast<std::uint8_t const *>(d)[i];
                    }
#else
                    std::memcpy(data(), d, s);
#endif
                }
            }

            bool valid(std::uint8_t const *e) const {
                std::uint8_t const *b{buffer_start()};
                std::intptr_t const whole_size_limit{(std::intptr_t)((std::uintptr_t)e - (std::uintptr_t)b)};
                if(whole_size_limit > 0) {
                    std::size_t const ss{n_size(*b)};
                    if((std::int64_t)(ss * 2) <= whole_size_limit) {
                        std::size_t s{size()};
                        if((std::int64_t)(ss * 2 + s) <= whole_size_limit) {
                            std::uint8_t const *bs_last_ptr{b + ss * 2 + s - 1};
                            std::size_t const bss{n_size(*bs_last_ptr)};
                            if(bss == ss) {
                                std::uint8_t buff[16]{};
                                std::uint8_t const *bs_ptr{b + ss + s};
                                if(ss > 1) { std::memcpy(buff, bs_ptr, ss); } else { buff[0] = *bs_ptr; }
                                std::size_t bs{};
                                if(ss > 1) { lins::bit_util::inplace_swap(buff, ss); }
                                mem_to_unumber(buff, bs);
                                if(bs == s) {
                                    return true;
                                }
                            }
                        }
                    }
                }
                return false;
            }

            size_surrounded_buffer *prev() const {
                std::uint8_t const *prev_last_byte_ptr{buffer_start() - 1};
                std::size_t prev_size_size{n_size(*prev_last_byte_ptr)};

                std::uint8_t const *prev_last_size_ptr{buffer_start() - prev_size_size};

                std::uint8_t buff[16];
                std::memcpy(buff, prev_last_size_ptr, prev_size_size);
                lins::bit_util::inplace_swap(buff, prev_size_size);

                std::size_t prev_size;
                mem_to_unumber(buff, prev_size);
                return reinterpret_cast<size_surrounded_buffer *>(const_cast<std::uint8_t *>(buffer_start()) - (prev_size_size * 2 + prev_size));
            }

            size_surrounded_buffer *next() const {
                return (size_surrounded_buffer *)(buffer_start() + (size_size() * 2 + size()));
            }

            std::int64_t as_number() const {
                std::size_t s{size()};
                if(s == sizeof(std::int64_t)) {
                    return *reinterpret_cast<const int64_t *>(data());
                } else if(s == sizeof(std::int32_t)) {
                    return static_cast<std::int64_t>(*reinterpret_cast<const std::int32_t *>(data()));
                } else if(s == sizeof(std::int16_t)) {
                    return static_cast<std::int64_t>(*reinterpret_cast<const std::int16_t *>(data()));
                } else if(s == sizeof(std::int8_t)) {
                    return static_cast<std::int64_t>(*reinterpret_cast<const std::int8_t *>(data()));
                }
                throw bad_serial_cast{"Invalid content for being a number"};
            }

            std::uint64_t as_unumber() const {
                std::size_t s{size()};
                if(s == sizeof(std::uint64_t)) {
                    return *reinterpret_cast<const std::uint64_t *>(data());
                } else if(s == sizeof(std::uint32_t)) {
                    return static_cast<std::uint64_t>(*reinterpret_cast<const std::uint32_t *>(data()));
                } else if(s == sizeof(std::uint16_t)) {
                    return static_cast<uint64_t>(*reinterpret_cast<const std::uint16_t *>(data()));
                } else if(s == sizeof(std::uint8_t)) {
                    return static_cast<std::uint64_t>(*reinterpret_cast<const std::uint8_t *>(data()));
                }
                throw bad_serial_cast{"Invalid content for being a number"};
            }

            std::string_view as_string() const {
                std::size_t s{size()};
                if(s >= sizeof(char)) {
                    return std::string_view{reinterpret_cast<const char *>(data()), s};
                } else {
                    throw bad_serial_cast{"Invalid content for being a string"};
                }
            }

            std::wstring_view as_wstring() const {
                std::size_t s{size()};
                if(s >= sizeof(wchar_t)) {
                    if(s % sizeof(wchar_t)) {
                        throw bad_serial_cast{"Invalid content for being a unicode string"};
                    }
                    return std::wstring_view{reinterpret_cast<wchar_t const *>(data()), s / sizeof(wchar_t)};
                } else {
                    return std::wstring_view{};
                }
            }

            std::vector<std::uint8_t> as_bytevec() const {
                if(size()) {
                    return std::vector<std::uint8_t>{
                        reinterpret_cast<std::uint8_t const *>(data()),
                        reinterpret_cast<std::uint8_t const *>(data()) + size()
                    };
                } else {
                    return std::vector<std::uint8_t>{};
                }
            }

            template<typename U>
            const U &as() const {
                if(size() != sizeof(U)) {
                    throw bad_serial_cast{"Invalid content cast in serialized data"};
                }
                return *reinterpret_cast<const U *>(data());
            }

            template<typename U>
            U &as() {
                if(size() != sizeof(U)) {
                    throw bad_serial_cast{"Invalid content cast in serialized data"};
                }
                return *reinterpret_cast<U *>(data());
            }

        private:
            std::uint8_t *buffer_start() { return reinterpret_cast<std::uint8_t *>(this); }
            std::uint8_t const *buffer_start() const { return reinterpret_cast<std::uint8_t const *>(this); }
        };


        class serialized_data_iter final/*: public std::iterator<std::bidirectional_iterator_tag, size_surrounded_buffer>*/ {
        public:
            serialized_data_iter():
                p_{nullptr},
                whole_data_{nullptr},
                data_size_{0}
            {
            }

            serialized_data_iter(const size_surrounded_buffer *x,
                                 const std::uint8_t *whole_data,
                                 std::size_t data_size):
                p_{x},
                whole_data_{whole_data},
                data_size_{data_size}
            {
            }

            serialized_data_iter(const serialized_data_iter &that) = default;

            serialized_data_iter &operator=(const serialized_data_iter &that) = default;

            serialized_data_iter(serialized_data_iter &&that) = default;

            serialized_data_iter &operator=(serialized_data_iter &&that) = default;

            ~serialized_data_iter() = default;

            bool valid() const {
                if(p_) {
                    return p_->valid(whole_data_ + data_size_);
                }
                return false;
            }

            serialized_data_iter &operator--() {
                if(reinterpret_cast<size_surrounded_buffer const *>(p_) != reinterpret_cast<size_surrounded_buffer const *>(whole_data_)) {
                    if(((std::uint8_t *)p_) > whole_data_) {
                        p_ = p_->prev();
                        return_into_bounds_if_needed();
                    }
                }
                return *this;
            }

            serialized_data_iter operator--(int) {
                serialized_data_iter tmp{*this};
                operator--();
                return tmp;
            }

            serialized_data_iter &operator++() {
                std::uint8_t const *e{whole_data_ + data_size_};
                if(((std::uint8_t *)p_) > e) {
                    p_ = reinterpret_cast<size_surrounded_buffer const *>(e);
                } else {
                    p_ = p_->next();
                    return_into_bounds_if_needed();
                }
                return *this;
            }

            serialized_data_iter operator++(int) {
                serialized_data_iter tmp{*this};
                operator++();
                return tmp;
            }

            serialized_data_iter operator+(std::int64_t incr) const {
                serialized_data_iter tmp{*this};

                if(incr < 0) {
                    for(std::int64_t i{0}; i < -incr; ++i) { --tmp; }
                } else {
                    for(std::int64_t i{0}; i < incr; ++i) { ++tmp; }
                }

                return tmp;
            }

            serialized_data_iter operator-(std::int64_t incr) const {
                serialized_data_iter tmp{*this};

                if(incr < 0) {
                    for(std::int64_t i{0}; i < -incr; i++) { ++tmp; }
                } else {
                    for(std::int64_t i{0}; i < incr; i++) { --tmp; }
                }

                return tmp;
            }

            serialized_data_iter &operator+=(std::int64_t incr) {
                *this = *this + incr;
                return *this;
            }

            serialized_data_iter &operator-=(std::int64_t incr) {
                *this = *this - incr;
                return *this;
            }

            bool operator<(const serialized_data_iter &rhs) const {
                return p_ < rhs.p_;
            }

            bool operator<=(const serialized_data_iter &rhs) const {
                return p_ <= rhs.p_;
            }

            bool operator>(const serialized_data_iter &rhs) const {
                return p_ > rhs.p_;
            }

            bool operator>=(const serialized_data_iter &rhs) const {
                return p_ >= rhs.p_;
            }

            bool operator==(const serialized_data_iter &rhs) const {
                return p_ == rhs.p_;
            }

            bool operator!=(const serialized_data_iter &rhs) const {
                return p_ != rhs.p_;
            }

            const size_surrounded_buffer &operator*() const {
                return *p_;
            }

            const size_surrounded_buffer *operator->() const {
                return p_;
            }

            operator bool() const {
                return !!p_;
            }

        private:
            void return_into_bounds_if_needed() {
                std::uint8_t const *e{whole_data_ + data_size_};
                if(((std::uint8_t *)p_) > e) {
                    p_ = reinterpret_cast<size_surrounded_buffer const *>(e);
                } else if(((std::uint8_t *)p_) < whole_data_) {
                    p_ = reinterpret_cast<size_surrounded_buffer const *>(whole_data_);
                }
            }

        private:
            size_surrounded_buffer const *p_{nullptr};
            std::uint8_t const *whole_data_{nullptr};
            std::size_t data_size_{0};
        };
    }


    class serializer final {
    public:
        typedef std::vector<std::uint8_t>::size_type size_type;
        typedef details_serial::serialized_data_iter iterator;
        typedef details_serial::serialized_data_iter const_iterator;

    public:
        serializer() = default;

        serializer(void const *data, std::size_t size) {
            if(data && size) {
                serialized_data_ = std::vector<std::uint8_t>{
                    reinterpret_cast<const std::uint8_t *>(data),
                    reinterpret_cast<const std::uint8_t *>(data) + size
                };
            }
        }

        serializer(const std::vector<std::uint8_t> &data):
            serialized_data_{data} {
        }

        serializer(const std::vector<std::uint8_t> &&data):
            serialized_data_{std::move(data)} {
        }

        serializer &operator=(std::vector<std::uint8_t> const &data) {
            serialized_data_ = data;
            return *this;
        }

        serializer &operator=(std::vector<std::uint8_t> &&data) {
            serialized_data_ = std::move(data);
            return *this;
        }

        std::vector<std::uint8_t> const &data_vec() const {
            return serialized_data_;
        }

        std::vector<std::uint8_t> &&take_vec() {
            return std::move(serialized_data_);
        }

        void reset() {
            serialized_data_.clear();
        }

        void clear() {
            serialized_data_.clear();
        }

        void reserve(size_type rsv_size) {
            serialized_data_.reserve(rsv_size);
        }

        size_type size() const {
            return serialized_data_.size();
        }

        std::uint8_t const *data() const {
            return serialized_data_.data();
        }

        void push_back(const void *d, std::size_t s) {
            std::size_t new_data_pos{size()};
            int s_size{details_serial::n_bytes(s)};
            serialized_data_.resize(new_data_pos + s + s_size * 2);
            details_serial::size_surrounded_buffer *buf{
                    (details_serial::size_surrounded_buffer *)
                        ((reinterpret_cast<char *>(serialized_data_.data())) + new_data_pos)};
            buf->set_data(d, s);
        }

        void push_back(const std::string &str) {
            push_back(str.c_str(), str.size());
        }

        void push_back(const char *str) {
            push_back(str, strlen(str));
        }

        void push_back(const std::wstring &str) {
            push_back(str.data(), str.size() * sizeof(wchar_t));
        }

        void push_back(const wchar_t *str) {
            push_back(str, (::wcslen(str)) * sizeof(wchar_t));
        }

        void push_back(const std::vector<std::uint8_t> &vec) {
            push_back(vec.data(), vec.size());
        }

        template<typename PUSHED_T>
        void push_back(const PUSHED_T &d) {
            if constexpr (sys_util::little_endian()) {
                push_back(&d, sizeof(d));
            } else {
                PUSHED_T bd{bit_util::swap_on_be<PUSHED_T>{d}.val};
                push_back(&bd, sizeof(bd));
            }
        }

        iterator begin() {
            iterator res{
                 reinterpret_cast<details_serial::size_surrounded_buffer *>(serialized_data_.data()),
                 serialized_data_.data(),
                 serialized_data_.size()
            };
            return res;
        }

        iterator end() {
            return iterator{
                reinterpret_cast<details_serial::size_surrounded_buffer *>(
                    serialized_data_.data() + serialized_data_.size()
                ),
                serialized_data_.data(),
                serialized_data_.size()
            };
        }

        const_iterator begin() const {
            return const_iterator{
                reinterpret_cast<details_serial::size_surrounded_buffer const *>(serialized_data_.data()),
                serialized_data_.data(),
                serialized_data_.size()
            };
        }

        const_iterator end() const {
            return const_iterator{
                reinterpret_cast<details_serial::size_surrounded_buffer const *>(
                    serialized_data_.data() + serialized_data_.size()
                ),
                serialized_data_.data(),
                serialized_data_.size()
            };
        }

        const_iterator cbegin() const {
            return begin();
        }

        const_iterator cend() const {
            return end();
        }

        template<typename PUSHED_T>
        serializer &operator<<(const PUSHED_T &v) {
            push_back(v);
            return *this;
        }

        serializer &operator<<(const std::string &str) {
            push_back(str);
            return *this;
        }

        serializer &operator<<(const char *str) {
            push_back(str);
            return *this;
        }

        serializer &operator<<(const std::wstring &str) {
            push_back(str);
            return *this;
        }

        serializer &operator<<(const wchar_t *str) {
            push_back(str);
            return *this;
        }

        serializer &operator<<(const std::vector<std::uint8_t> &vec) {
            push_back(vec);
            return *this;
        }

    private:
        std::vector<std::uint8_t> serialized_data_{};
    };


    class serial_reader final {
    public:
        typedef std::vector<std::uint8_t>::size_type size_type;
        typedef details_serial::serialized_data_iter const_iterator;

    public:
        serial_reader() = default;

        serial_reader(void const *data, std::size_t data_size):
            data_{reinterpret_cast<const std::uint8_t *>(data)},
            data_size_{data_size}
        {
        }

        void assign(void const *data, std::size_t data_size) {
            data_ = reinterpret_cast<const std::uint8_t *>(data);
            data_size_ = data_size;
        }

        size_type size() const {
            return data_size_;
        }

        std::uint8_t const *data() const {
            return data_;
        }

        const_iterator begin() const {
            return const_iterator{reinterpret_cast<const details_serial::size_surrounded_buffer *>(data()), data(), size()};
        }

        const_iterator end() const {
            return const_iterator{reinterpret_cast<const details_serial::size_surrounded_buffer *>(data() + size()), data(), size()};
        }

        const_iterator cbegin() const {
            return const_iterator{reinterpret_cast<const details_serial::size_surrounded_buffer *>(data()), data(), size()};
        }

        const_iterator cend() const {
            return const_iterator{reinterpret_cast<const details_serial::size_surrounded_buffer *>(data() + size()), data(), size()};
        }

    private:
        const std::uint8_t *data_{nullptr};
        std::size_t data_size_{0};
    };
#pragma pack(pop)

    class concatenator {
    public:
        template<typename T>
        void add(T const &v) { std::uint8_t const *v_ptr{(std::uint8_t const *)&v}; v_.insert(v_.end(), v_ptr, v_ptr + sizeof(T)); }
        void add(std::string const &v) { v_.insert(v_.end(), v.begin(), v.end()); }
        void add(std::string_view v) { v_.insert(v_.end(), v.begin(), v.end()); }
        void add(char const *v) { add(std::string{v}); }
        template<typename T>
        void add(std::vector<T> const &v) { for(auto &&item: v) { add(item); } }
        template<typename T, std::size_t N>
        void add(std::array<T, N> const &v) { for(auto &&item: v) { add(item); } }
        void clear() { v_.clear(); }
        std::vector<std::uint8_t> const &data() const { return v_; }
        std::vector<std::uint8_t> &data() { return v_; }

        template<typename T>
        friend concatenator &operator<<(concatenator &cat, T const v) { cat.add(v); return cat; }
        friend concatenator &operator<<(concatenator &cat, std::string const &s) { cat.add(s); return cat; }
        friend concatenator &operator<<(concatenator &cat, char *v) { cat.add(v); return cat; }
        template<typename T>
        friend concatenator &operator<<(concatenator &cat, std::vector<T> const &v) { cat.add(v); return cat; }
        template<typename T, std::size_t N>
        friend concatenator &operator<<(concatenator &cat, std::array<T, N> const &v) { for(auto &&item: v) { cat.add(item); } return cat; }

    private:
        std::vector<std::uint8_t> v_{};
    };

}

#undef USE_MANUAL_MEMORY_COPYING
