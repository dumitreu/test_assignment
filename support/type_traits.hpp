#pragma once

namespace lins {

    template <typename T>
    struct basic_type {
        typedef T type;
    };

    template <typename T>
    struct basic_type<T const > {
        typedef T type;
    };

    template <typename T>
    struct basic_type<T volatile> {
        typedef T type;
    };

    template <typename T>
    struct basic_type<T volatile const> {
        typedef T type;
    };

    template <typename T>
    struct basic_type<T const &> {
        typedef T type;
    };

    template <typename T>
    struct basic_type<T volatile const &> {
        typedef T type;
    };

    template <typename T>
    struct basic_type<T &> {
        typedef T type;
    };

    template <typename T>
    struct basic_type<T volatile &> {
        typedef T type;
    };

    template <typename T>
    struct basic_type<T &&> {
        typedef T type;
    };

    template<class T>
    using basic_t = typename basic_type<T>::type;

}
