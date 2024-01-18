#pragma once

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include "type_traits.hpp"

namespace lins {

    class bad_any_cast : public std::bad_cast {
    public:
        virtual const char *what() const noexcept {
            return "bad_any_cast";
        }
    };

    class any {
    public:
        any() = default;

        any(const any &item) {
            if(item.data) {
                item.data->copy_to(data);
            }
        }

        any &operator=(const any &item) {
            if(&item != this) {
                any{item}.swap(*this);
            }
            return *this;
        }

        any(any &&item) noexcept: data(std::move(item.data)) {
        }

        any &operator=(any &&item) noexcept {
            if(this != &item) {
                data = std::move(item.data);
            }
            return *this;
        }

        ~any() = default;

        void swap(any &item) {
            data.swap(item.data);
        }

        template <typename T>
        any(const T &item) {
            typedef typename basic_type<T>::type U;
            data.reset(new derived<U>{item});
        }

        void clear() {
            data.reset();
        }

        template <typename T>
        bool contains() const {
            typedef typename basic_type<T>::type U;
            return dynamic_cast<derived<U> *>(data.get()) != 0;
        }

        bool is_empty() const {
            return data.get() == 0;
        }

        template <typename T>
        T &cast_to() {
            typedef typename basic_type<T>::type U;
            derived<U> *d = dynamic_cast<derived<U> *>(data.get());
            if(d == 0) {
                throw bad_any_cast();
            }
            return d->item;
        }

        template <typename T>
        const T &cast_to() const {
            typedef typename basic_type<T>::type U;
            derived<U> *d = dynamic_cast<derived<U> *>(data.get());
            if(d == 0) {
                throw bad_any_cast();
            }
            return d->item;
        }

        template <typename T>
        T &as() {
            typedef typename basic_type<T>::type U;
            derived<U> *d = dynamic_cast<derived<U> *>(data.get());
            if(d == 0) {
                throw bad_any_cast();
            }
            return d->item;
        }

        template <typename T>
        const T &as() const {
            typedef typename basic_type<T>::type U;
            derived<U> *d = dynamic_cast<derived<U> *>(data.get());
            if(d == 0) {
                throw bad_any_cast();
            }
            return d->item;
        }

        template <typename T>
        T *as_ptr() noexcept {
            typedef typename basic_type<T>::type U;
            derived<U> *d = dynamic_cast<derived<U> *>(data.get());
            return d ? &(d->item) : 0;
        }

        template <typename T>
        const T *as_ptr() const noexcept {
            typedef typename basic_type<T>::type U;
            derived<U> *d = dynamic_cast<derived<U> *>(data.get());
            return d ? &(d->item) : 0;
        }

        template <typename T>
        bool is_of_type() const noexcept {
            typedef typename basic_type<T>::type U;
            return dynamic_cast<derived<U> *>(data.get()) != 0;
        }

        template <typename T>
        T &get() {
            typedef typename basic_type<T>::type U;
            derived<U> *d = dynamic_cast<derived<U> *>(data.get());
            if(d == 0) {
                d = new derived<U>();
                data.reset(d);
            }

            return d->item;
        }

        void *ptr() noexcept {
            return data ? data->ptr() : nullptr;
        }

        const void *ptr() const noexcept {
            return data ? data->ptr() : nullptr;
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
            T item;
            derived() {}
            derived(const T &val): item(val) {}
            derived(T &&val) noexcept: item(std::move(val)) {}
            derived(derived &&that) noexcept: item(std::move(that.item)) {}
            derived &operator=(derived &&that) noexcept { item = std::move(that.item); return *this;}
            void copy_to (std::unique_ptr<base> &dest) const override { dest.reset(new derived<T>(item)); }
            void *ptr() override { return &item; }
            const void *ptr() const override { return &item; }
        };

        std::unique_ptr<base> data{nullptr};
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
