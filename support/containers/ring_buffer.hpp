#pragma once

#include "../commondefs.hpp"

namespace lins {

    template<typename T, std::size_t CAPACITY>
    class ring_buffer {
        static_assert(CAPACITY > 0, "CAPACITY must be non-zero number");
        class iter {
        public:
            using value_type = T;
            using difference_type = ptrdiff_t;
            using pointer = T *;
            using reference = T &;

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
            using value_type = T;
            using difference_type = ptrdiff_t;
            using pointer = T *;
            using reference = T &;

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
                new(buff() + wrp_idx(i)) T(std::move(that[i]));
                ++size_;
            }
            that.clear();
        }

        ring_buffer &operator=(ring_buffer const &that) {
            if(&that != this) {
                clear();
                for(std::size_t i{0}; i < that.size_; ++i) {
                    new(buff() + wrp_idx(i)) T(that[i]);
                    ++size_;
                }
            }
            return *this;
        }

        ring_buffer &operator=(ring_buffer &&that) {
            if(&that != this) {
                clear();
                for(std::size_t i{0}; i < that.size_; ++i) {
                    new(buff() + wrp_idx(i)) T(std::move(that[i]));
                    ++size_;
                }
                that.clear();
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
            return CAPACITY;
        }

        std::size_t size() const noexcept {
            return size_;
        }

        void push_back(T const &val) {
            if(size_ == CAPACITY) {
                if constexpr (std::is_copy_assignable<T>::value) {
                    buff()[pos_] = val;
                } else {
                    buff()[pos_].~T();
                    new(buff() + pos_) T(val);
                }
                pos_ = wrp_idx(1);
            } else {
                new(buff() + wrp_idx(size_++)) T(val);
            }
        }

        void push_back(T &&val) {
            if(size_ == CAPACITY) {
                if constexpr (std::is_move_assignable<T>::value) {
                    buff()[pos_] = std::move(val);
                } else {
                    buff()[pos_].~T();
                    new(buff() + pos_) T(std::move(val));
                }
                pos_ = wrp_idx(1);
            } else {
                new(buff() + wrp_idx(size_++)) T(std::move(val));
            }
        }

        template<typename ...ARGS>
        void emplace_back(ARGS &&...args) {
            if(size_ == CAPACITY) {
                if constexpr (std::is_move_assignable<T>::value) {
                    buff()[pos_] = T{std::forward<ARGS>(args)...};
                } else {
                    buff()[pos_].~T();
                    new(buff() + pos_) T(std::forward<ARGS>(args)...);
                }
                pos_ = wrp_idx(1);
            } else {
                new(buff() + wrp_idx(size_++)) T(std::forward<ARGS>(args)...);
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
            if(!empty()) {
                return buff()[pos_];
            }
            throw std::runtime_error{"buffer is empty"};
        }

        T &back() {
            if(!empty()) {
                return buff()[wrp_idx(size_ - 1)];
            }
            throw std::runtime_error{"buffer is empty"};
        }

        T const &front() const {
            if(!empty()) {
                return buff()[pos_];
            }
            throw std::runtime_error{"buffer is empty"};
        }

        T const &back() const {
            if(!empty()) {
                return buff()[wrp_idx(size_ - 1)];
            }
            throw std::runtime_error{"buffer is empty"};
        }

        T pop_front() {
            if(!empty()) {
                T res{std::move(buff()[pos_])};
                buff()[pos_].~T();
                pos_ = wrp_idx(1);
                --size_;
                return res;
            } else {
                throw std::runtime_error{"the buffer is empty"};
            }
        }

        T pop_back() {
            if(!empty()) {
                T res{std::move(buff()[wrp_idx(size_ - 1)])};
                buff()[wrp_idx(--size_)].~T();
                return res;
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
            return (pos_ + indx) % CAPACITY;
        }

    private:
        std::uint8_t buf_[sizeof(T) * CAPACITY]{0};
        std::size_t pos_{0};
        std::size_t size_{0};
    };

}
