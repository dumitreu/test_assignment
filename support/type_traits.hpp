#pragma once

namespace lins {

    template <typename T>
    struct basic_type {
        typedef T type;
    };

    template <typename T>
    struct basic_type<const T> {
        typedef T type;
    };

    template <typename T>
    struct basic_type<const T &> {
        typedef T type;
    };

    template <typename T>
    struct basic_type<volatile const T &> {
        typedef T type;
    };

    template <typename T>
    struct basic_type<T &> {
        typedef T type;
    };

    template <typename T>
    struct basic_type<volatile T &> {
        typedef T type;
    };

    template <typename T>
    struct basic_type<volatile T> {
        typedef T type;
    };

    template <typename T>
    struct basic_type<volatile const T> {
        typedef T type;
    };

}
