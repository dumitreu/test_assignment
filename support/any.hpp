#pragma once

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include "type_traits.hpp"

namespace lins {

    class bad_any_cast : public std::bad_cast {
    public:
        virtual char const *what() const noexcept {
            return "bad_any_cast";
        }
    };

    class any {
    public:
        any() = default;

        any(any const &that) {
            if(that.data_) {
                that.data_->copy_to(data_);
            }
        }

        any &operator=(any const &that) {
            if(&that != this) {
                any{that}.swap(*this);
            }
            return *this;
        }

        any(any &&that) noexcept:
            data_{std::move(that.data_)}
        {
        }

        any &operator=(any &&that) noexcept {
            if(this != &that) {
                data_ = std::move(that.data_);
            }
            return *this;
        }

        ~any() = default;

        void swap(any &that) {
            data_.swap(that.data_);
        }

        template <typename T>
        any(T const &val):
            data_{std::make_unique<derived<basic_t<T>>>(val)}
        {
        }

        template <typename T>
        any &operator=(T const &val) {
            any{val}.swap(*this);
            return *this;
        }

        template <typename T>
        any(T &&val) noexcept:
            data_{std::make_unique<derived<basic_t<T>>>(std::forward<T>(val))}
        {
        }

        template <typename T>
        any &operator=(T &&val) noexcept {
            any{std::forward<T>(val)}.swap(*this);
            return *this;
        }

        void clear() {
            data_.reset();
        }

        template <typename T>
        bool contains() const {
            return !!dynamic_cast<derived<basic_t<T>> *>(data_.get());
        }

        bool empty() const {
            return !data_;
        }

        template <typename T>
        T &cast_to() {
            derived<basic_t<T>> *d{dynamic_cast<derived<basic_t<T>> *>(data_.get())};
            if(!d) {
                throw bad_any_cast{};
            }
            return d->item_;
        }

        template <typename T>
        T const &cast_to() const {
            derived<basic_t<T>> *d{dynamic_cast<derived<basic_t<T>> *>(data_.get())};
            if(!d) {
                throw bad_any_cast{};
            }
            return d->item_;
        }

        template <typename T>
        T &as() {
            return cast_to<T>();
        }

        template <typename T>
        T const &as() const {
            return cast_to<T>();
        }

        template <typename T>
        T *as_ptr() noexcept {
            derived<basic_t<T>> *d{dynamic_cast<derived<basic_t<T>> *>(data_.get())};
            return d ? &(d->item_) : 0;
        }

        template <typename T>
        T const *as_ptr() const noexcept {
            derived<basic_t<T>> *d{dynamic_cast<derived<basic_t<T>> *>(data_.get())};
            return d ? &(d->item_) : 0;
        }

        template <typename T>
        bool is_of_type() const noexcept {
            return dynamic_cast<derived<basic_t<T>> *>(data_.get()) != nullptr;
        }

        template <typename T>
        T &get_or_create() {
            derived<basic_t<T>> *d{dynamic_cast<derived<basic_t<T>> *>(data_.get())};
            if(d == nullptr) {
                data_ = std::make_unique<derived<basic_t<T>>>();
                d = dynamic_cast<derived<basic_t<T>> *>(data_.get());
            }
            return d->item_;
        }

        void *ptr() noexcept {
            return data_ ? data_->ptr() : nullptr;
        }

        const void *ptr() const noexcept {
            return data_ ? data_->ptr() : nullptr;
        }

    private:
        struct base {
            virtual ~base() = default;
            virtual void copy_to(std::unique_ptr<base> &dest) const = 0;
            virtual void *ptr() = 0;
            virtual const void *ptr() const = 0;
        };

        template <typename T>
        struct derived: public base {
            derived() {}
            derived(T const &val): item_{val} {}
            derived(T &&val) noexcept: item_{std::move_if_noexcept(val)} {}
            derived(derived &&that) noexcept: item_{std::move_if_noexcept(that.item_)} {}
            derived &operator=(derived &&that) noexcept { item_ = std::move_if_noexcept(that.item_); return *this;}
            void copy_to (std::unique_ptr<base> &dest) const override { dest = std::make_unique<derived<T>>(item_); }
            void *ptr() override { return &item_; }
            void const *ptr() const override { return &item_; }

            T item_{};
        };

        std::unique_ptr<base> data_{nullptr};
    };

    inline void swap(any &a, any &b) {
        a.swap(b);
    }

    template <typename T>
    T &any_cast(any &a) {
        return a.as<T>();
    }

    template <typename T>
    const T &any_cast(const any &a) {
        return a.as<T>();
    }

}
