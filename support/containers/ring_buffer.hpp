#pragma once

#include "../commondefs.hpp"

namespace lins {

    template<typename T, std::size_t SIZE>
    class ring_buffer {
        static_assert(SIZE > 0, "SIZE must be non-zero number");
        class iter {
        public:
            typedef T value_type;
            typedef ptrdiff_t  difference_type;
            typedef T *pointer;
            typedef T &reference;

            iter(iter const &) = default;
            iter(iter &&) = default;
            iter &operator=(iter const &) = default;
            iter &operator=(iter &&) = default;
            ~iter() = default;

            iter(ring_buffer *owner_ptr = nullptr, std::size_t curr_pos = static_cast<std::size_t>(-1)):
                owner_ptr_{owner_ptr}, curr_pos_{owner_ptr_ && curr_pos < owner_ptr_->size_ ? curr_pos : static_cast<std::size_t>(-1)} {
            }

            iter &operator++() {
                if(owner_ptr_ && curr_pos_ != static_cast<std::size_t>(-1) && curr_pos_ < owner_ptr_->size_) {
                    ++curr_pos_;
                    if(curr_pos_ >= owner_ptr_->size_) {
                        curr_pos_ = static_cast<std::size_t>(-1);
                    }
                }
                return *this;
            }

            iter operator++(int) {
                iter retval{*this};
                ++(*this);
                return retval;
            }

            bool operator==(iter other) const {
                return owner_ptr_ == other.owner_ptr_ && curr_pos_ == other.curr_pos_;
            }

            bool operator!=(iter other) const {
                return !(*this == other);
            }

            T &operator*() {
                if(owner_ptr_ && curr_pos_ != static_cast<std::size_t>(-1)) {
                    if(curr_pos_ < owner_ptr_->size_) {
                        return (*owner_ptr_)[curr_pos_];
                    } else {
                        throw std::range_error{"iterator out of range"};
                    }
                } else {
                    throw std::runtime_error{"invalid iterator"};
                }
            }

            T const &operator*() const {
                if(owner_ptr_ && curr_pos_ != static_cast<std::size_t>(-1)) {
                    if(curr_pos_ < owner_ptr_->size_) {
                        return (*owner_ptr_)[curr_pos_];
                    } else {
                        throw std::range_error{"iterator out of range"};
                    }
                } else {
                    throw std::runtime_error{"invalid iterator"};
                }
            }

        private:
            ring_buffer *owner_ptr_;
            std::size_t curr_pos_{static_cast<std::size_t>(-1)};
        };

        class const_iter {
        public:
            typedef T value_type;
            typedef ptrdiff_t  difference_type;
            typedef T *pointer;
            typedef T &reference;

            const_iter(const_iter const &) = default;
            const_iter(const_iter &&) = default;
            const_iter &operator=(const_iter const &) = default;
            const_iter &operator=(const_iter &&) = default;
            ~const_iter() = default;

            const_iter(ring_buffer const *owner_ptr = nullptr, std::size_t curr_pos = static_cast<std::size_t>(-1)):
                owner_ptr_{owner_ptr}, curr_pos_{owner_ptr_ && curr_pos < owner_ptr_->size_ ? curr_pos : static_cast<std::size_t>(-1)} {
            }

            const_iter &operator++() {
                if(owner_ptr_ && curr_pos_ != static_cast<std::size_t>(-1) && curr_pos_ < owner_ptr_->size_) {
                    ++curr_pos_;
                    if(curr_pos_ >= owner_ptr_->size_) {
                        curr_pos_ = static_cast<std::size_t>(-1);
                    }
                }
                return *this;
            }

            const_iter operator++(int) {
                const_iter retval{*this};
                ++(*this);
                return retval;
            }

            bool operator==(const_iter other) const {
                return owner_ptr_ == other.owner_ptr_ && curr_pos_ == other.curr_pos_;
            }

            bool operator!=(const_iter other) const {
                return !(*this == other);
            }

            T const &operator*() const {
                if(owner_ptr_ && curr_pos_ != static_cast<std::size_t>(-1)) {
                    if(curr_pos_ < owner_ptr_->size_) {
                        return (*owner_ptr_)[curr_pos_];
                    } else {
                        throw std::range_error{"iterator out of range"};
                    }
                } else {
                    throw std::runtime_error{"invalid iterator"};
                }
            }

        private:
            ring_buffer const *owner_ptr_;
            std::size_t curr_pos_{static_cast<std::size_t>(-1)};
        };

    public:
        using iterator = iter;
        using const_iterator = const_iter;

        ring_buffer() = default;

        ~ring_buffer() noexcept {
            clear();
        }

        ring_buffer(ring_buffer const &that) {
            for(std::size_t i{0}; i < that.size_; ++i) {
                push_back(that[i]);
            }
        }

        ring_buffer(ring_buffer &&that) {
            for(std::size_t i{0}; i < that.size_; ++i) {
                new(buff() + wrp_idx(i)) T{};
                std::swap(buff()[wrp_idx(i)], that[i]);
                ++size_;
            }
            that.size_ = 0;
        }

        ring_buffer &operator=(ring_buffer const &that) {
            if(&that != this) {
                clear();
                for(std::size_t i{0}; i < that.size_; ++i) {
                    new(buff() + wrp_idx(i)) T{};
                    buff()[wrp_idx(i)] = that.buff()[that.wrp_idx(i)];
                    ++size_;
                }
            }
            return *this;
        }

        ring_buffer &operator=(ring_buffer &&that) {
            if(&that != this) {
                clear();
                pos_ = that.pos_;
                for(std::size_t i{0}; i < that.size_; ++i) {
                    buff()[wrp_idx(i)] = std::move(that.buff()[that.wrp_idx(i)]);
                    ++size_;
                }
                that.size_ = 0;
                that.pos_ = 0;
            }
            return *this;
        }

        T &operator[](std::size_t indx) {
            if(indx < size()) {
                return buff()[wrp_idx(indx)];
            } else {
                throw std::range_error{"index out of bounds"};
            }
        }

        T const &operator[](std::size_t indx) const {
            if(indx < size()) {
                return buff()[wrp_idx(indx)];
            } else {
                throw std::range_error{"index out of bounds"};
            }
        }

        std::size_t constexpr capacity() const noexcept {
            return SIZE;
        }

        std::size_t size() const noexcept {
            return size_;
        }

        void push_back(T const &val) {
            if(size_ == SIZE) {
                buff()[pos_].~T();
            }
            new(buff() + wrp_idx(size_++)) T{val};
            if(size_ > SIZE) {
                pos_ = wrp_idx(1);
                size_ = SIZE;
            }
        }

        void push_back(T &&val) {
            if(size_ == SIZE) {
                buff()[pos_].~T();
            }
            new(buff() + wrp_idx(size_)) T{};
            std::swap(buff()[wrp_idx(size_)], val);
            ++size_;
            if(size_ > SIZE) {
                pos_ = (pos_ + 1) % SIZE;
                size_ = SIZE;
            }
        }

        template<typename ...ARGS>
        void emplace_back(ARGS &&...args) {
            if(size_ == SIZE) {
                buff()[pos_].~T();
            }
            new(buff() + wrp_idx(size_++)) T{std::forward<ARGS>(args)...};
            if(size_ > SIZE) {
                pos_ = wrp_idx(1);
                size_ = SIZE;
            }
        }

        void clear() noexcept {
            for(std::size_t i{0}; i < size_; ++i) {
                buff()[wrp_idx(i)].~T();
            }
            pos_ = 0;
            size_ = 0;
        }

        iterator begin() {
            return iterator{this, 0};
        }

        iterator end() {
            return iterator{this, static_cast<std::size_t>(-1)};
        }

        const_iterator begin() const {
            return const_iterator{this, 0};
        }

        const_iterator end() const {
            return const_iterator{this, static_cast<std::size_t>(-1)};
        }

        const_iterator cbegin() const {
            return const_iterator{this, 0};
        }

        const_iterator cend() const {
            return const_iterator{this, static_cast<std::size_t>(-1)};
        }

        bool empty() const noexcept {
            return size() == 0;
        }

        T &front() {
            if(size()) {
                return buff()[pos_];
            }
            throw std::runtime_error{"buffer is empty"};
        }

        T &back() {
            if(size()) {
                return buff()[wrp_idx(size_ - 1)];
            }
            throw std::runtime_error{"buffer is empty"};
        }

        T const &front() const {
            if(size()) {
                return buff()[pos_];
            }
            throw std::runtime_error{"buffer is empty"};
        }

        T const &back() const {
            if(size()) {
                return buff()[wrp_idx(size_ - 1)];
            }
            throw std::runtime_error{"buffer is empty"};
        }

        T pop_front() {
            if(size() > 0) {
                size_--;
                auto org_pos{pos_};
                pos_ = wrp_idx(1);
                return std::move(buff()[org_pos]);
            } else {
                throw std::runtime_error{"the buffer is empty"};
            }
        }

        T pop_back() {
            if(size() > 0) {
                size_--;
                return std::move(buff()[wrp_idx(size_)]);
            } else {
                throw std::runtime_error{"the buffer is empty"};
            }
        }

    private:
        T *buff() {
            return reinterpret_cast<T *>(buf_);
        }

        T const *buff() const {
            return reinterpret_cast<T const *>(buf_);
        }

        std::size_t wrp_idx(std::size_t indx) const {
            return (pos_ + indx) % SIZE;
        }

        std::uint8_t buf_[sizeof(T) * SIZE]{0};
        std::size_t pos_{0};
        std::size_t size_{0};
    };

}
