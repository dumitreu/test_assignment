#pragma once
#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include <cstdio>
#include <ctime>
#include <cstdarg>
#include <ios>
#include <iomanip>
#include <fstream>
#include <functional>
#include <future>
#include <string>
#include <cstring>
#include <cwctype>
#include <vector>
#include <deque>
#include <queue>
#include <list>
#include <map>
#include <stack>
#include <limits>
#include <ctime>
#include <set>
#include <bitset>
#include <typeinfo>
#include <memory>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <iterator>
#include <complex>
#include <type_traits>
#if (__cplusplus >= 201103L)
#include <valarray>
#include <unordered_map>
#include <array>
#include <locale>
#include <cassert>
#include <cstdint>
#include <thread>
#include <mutex>
#include <atomic>
#include <random>
#include <regex>
#endif
#if (__cplusplus >= 201400L)
#include <shared_mutex>
#endif
#if (__cplusplus >= 201703L)
#include <optional>
#include <string_view>
#include <filesystem>
#endif
#if (__cplusplus >= 202002L)
#include <concepts>
#endif
#include <cstdio>
#include <cmath>
#include <streambuf>
#include <istream>
#include <sstream>
#include <ostream>
#include <exception>
#include <stdexcept>
#include <cassert>

// Check GCC
#if defined(__GNUC__)
#if defined(__x86_64__) || defined(__ppc64__)
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

#if defined(_WIN64)
    #define PLATFORM_WINDOWS
    #include <WinSock2.h>
    #include <ws2tcpip.h>
    #ifndef ENVIRONMENT64
        #define ENVIRONMENT64
    #endif
#elif defined(_WIN32)
    #define PLATFORM_WINDOWS
    #include <WinSock2.h>
    #include <ws2tcpip.h>
    #ifndef ENVIRONMENT32
        #define ENVIRONMENT32
    #endif
#elif defined(__APPLE__)
    #define PLATFORM_APPLE
    #include <fcntl.h>
    #include <unistd.h>
    #include <stdlib.h>
//    #if TARGET_OS_IPHONE
//    #elif TARGET_IPHONE_SIMULATOR
//    #elif TARGET_OS_MAC
//    #else
//    #endif
#elif defined(__ANDROID__)
#define PLATFORM_ANDROID
    #if !defined(_XOPEN_SOURCE) || _XOPEN_SOURCE < 500
    #define _XOPEN_SOURCE 500
    #endif
    #include <signal.h>
    #include <time.h>
    #include <sys/wait.h>
#define _POSIX1_SOURCE 2
    #include <unistd.h>
    #include <semaphore.h>
    #include <assert.h>
    #include <errno.h>
    #include <fcntl.h>
    #include <stdint.h>
    #include <dirent.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <sys/time.h>
    #include <sys/socket.h>
    #include <sys/ioctl.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <ftw.h>
    #include <sys/poll.h>
    #include <pthread.h>
    #include <arpa/inet.h>
    #include <sys/utsname.h>
    #include <netinet/ip.h>
    #include <netinet/ip_icmp.h>
    #include <sys/random.h>
#elif defined(__linux__)
    #define PLATFORM_LINUX
    #if !defined(_XOPEN_SOURCE) || _XOPEN_SOURCE < 500
    #define _XOPEN_SOURCE 500
    #endif
    #include <signal.h>
    #include <time.h>
    #include <sys/wait.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <getopt.h>
#define _POSIX1_SOURCE 2
    #include <unistd.h>
    #include <semaphore.h>
    #include <assert.h>
    #include <errno.h>
    #include <fcntl.h>
    #include <stdint.h>
    #include <dirent.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <sys/time.h>
    #include <sys/socket.h>
    #include <sys/ioctl.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <ftw.h>
    #include <sys/poll.h>
    #include <pthread.h>
    #include <arpa/inet.h>
    #include <sys/utsname.h>
    #include <netinet/ip.h>
    #include <netinet/ip_icmp.h>
    #include <sys/random.h>
#elif defined(__unix) // all unices not caught above
    #define PLATFORM_UNIXISH
#elif defined(__posix)
    #define PLATFORM_POSIX
#endif

typedef double real;

#define BEGIN_NAMESPACE(ns_name) namespace ns_name {
#define END_NAMESPACE(ns_name) } // ns_name

#define EXCEPTION_CLASS(CLASS_NAME, MESSAGE) \
    class CLASS_NAME: public std::exception { \
    public: \
      CLASS_NAME() throw() { } \
      virtual ~CLASS_NAME() throw() {} \
      virtual const char* what() const throw() { \
          return MESSAGE; \
      } \
    };

#define DEFINE_RUNTIME_EXCEPTION_CLASS(CLASS_NAME) class CLASS_NAME: public std::runtime_error { \
public: \
    CLASS_NAME(const std::string &msg): std::runtime_error(msg) { \
    } \
};

#define RT_ERROR_CLASS(CLASS_NAME) \
    class CLASS_NAME: public std::runtime_error { \
    public: \
      CLASS_NAME(const std::string &arg): std::runtime_error{arg} {} \
    };

#define RT_ERROR_CLASS_MSG(CLASS_NAME, MESSAGE) \
    class CLASS_NAME: public std::runtime_error { \
    public: \
      CLASS_NAME(): std::runtime_error{MESSAGE} {} \
    }

BEGIN_NAMESPACE(lins)

typedef int64_t off64_t;
typedef uint64_t pos64_t;
typedef off64_t off_t;
typedef pos64_t pos_t;

#ifdef PLATFORM_WINDOWS
#if defined(ENVIRONMENT64)
typedef int64_t ssize_t;
#elif defined(ENVIRONMENT32)
typedef int32_t ssize_t;
#endif
#endif

using bytevec = std::vector<std::uint8_t>;

inline int little_endian(void) {
    union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] != 1;
}

template<typename T, int size>
int array_size(T(&)[size]) {
    return size;
}

template<typename T>
T clamp(T const &value, T const &low, T const &high) {
    return value < low ? low : high < value ? high : value;
}

template<typename T, typename FUNCTOR>
class scoped_constructor_destructor final {
public:
    scoped_constructor_destructor(T &obj, FUNCTOR c, FUNCTOR d): ref{obj}, dref{d} {
        c(obj);
    }

    ~scoped_constructor_destructor() {
        dref(ref);
    }

private:
    T &ref;
    FUNCTOR dref;
};

template<typename T, typename FUNCTOR>
class scoped_finalizer final {
public:
    scoped_finalizer(T &obj, FUNCTOR const &d): ref{obj}, dref{d} {}
    scoped_finalizer(T &obj, FUNCTOR &&d): ref{obj}, dref{std::move(d)} {}
    scoped_finalizer(scoped_finalizer const &) = delete;
    scoped_finalizer &operator=(scoped_finalizer const &) = delete;
    scoped_finalizer(scoped_finalizer &&) = delete;
    scoped_finalizer &operator=(scoped_finalizer &&) = delete;
    ~scoped_finalizer() { dref(ref); }

private:
    T &ref;
    FUNCTOR dref;
};

template<typename FUNCTOR>
class shut_on_destroy final {
public:
    shut_on_destroy(FUNCTOR const &d): dref{d} {}
    shut_on_destroy(FUNCTOR &&d): dref{std::move(d)} {}
    shut_on_destroy(shut_on_destroy const &) = delete;
    shut_on_destroy &operator=(shut_on_destroy const &) = delete;
    shut_on_destroy(shut_on_destroy &&) = delete;
    shut_on_destroy &operator=(shut_on_destroy &&) = delete;
    ~shut_on_destroy() { dref(); }

private:
    FUNCTOR dref;
};

template<typename CONTAINER_T>
std::string container_to_string(CONTAINER_T const &c, std::string const &lp = "{",  std::string const &rp = "}") {
    std::stringstream ss{};
    std::string sep{};
    ss << lp;
    for(auto &&i: c) {
        ss << sep << i;
        if(sep.empty()) { sep = ", "; }
    }
    ss << rp;
    return ss.str();
}

template<typename T>
class comparizons final {
public:
    comparizons(T v): val_(v) {
    }

    bool between(T l, T r) const {
        return (l < r) ? ((val_ > l) && (val_ < r)) : ((val_ > r) && (val_ < l));
    }

    bool in(T l, T r) const {
        return (l < r) ? ((val_ >= l) && (val_ <= r)) : ((val_ >= r) && (val_ <= l));
    }

    T clamp(T l, T r) const {
        return val_ < l ? l : (val_ > r ? r : val_);
    }

private:
    const T val_;
};

template<typename T, typename FUNCTOR>
class on_changed_in_scope final {
public:
    on_changed_in_scope(const T &v): start_value(v), value_ref(v) {
    }

    ~on_changed_in_scope() {
        if(start_value != value_ref) {
            f();
        }
    }

private:
    T start_value;
    const T &value_ref;
    FUNCTOR f;
};

template<typename T>
class variable_value_tracker final {
public:
    variable_value_tracker(const T &ref): val_{ref}, ref_{ref} {
    }

    bool changed() const {
        return val_ != ref_;
    }

    operator bool() const {
        return val_ != ref_;
    }

private:
    T val_;
    const T &ref_;
};

END_NAMESPACE(lins)

#define TERNOP(c, l, r) ((c) ? (l) : (r))

#define INLINE_GETTER_SETTER(TYPE, NAME, FIELD) \
    TYPE const &NAME() const { \
        return FIELD; \
    } \
    void set_ ## NAME(TYPE const &f) { \
        FIELD = f; \
    }

#define INLINE_GETTER_SETTER_VAL(TYPE, NAME, FIELD) \
    TYPE NAME() const { \
        return FIELD; \
    } \
    void set_ ## NAME(TYPE f) { \
        FIELD = f; \
    }

#define INLINE_GETTER(TYPE, NAME, FIELD) \
    TYPE const &NAME() const { \
        return FIELD; \
    }

#define INLINE_SETTER(TYPE, NAME, FIELD) \
    void set_ ## NAME(TYPE const &f) { \
        FIELD = f; \
    }

#define INLINE_GETTER_VAL(TYPE, NAME, FIELD) \
    TYPE NAME() const { \
        return FIELD; \
    }

#define INLINE_SETTER_BYVAL(TYPE, NAME, FIELD) \
    void set_ ## NAME(TYPE f) { \
        FIELD = f; \
    }


#define INLINE_FAKE_GETTER_SETTER(TYPE, NAME, RET) \
    TYPE NAME() const { \
        return RET; \
    } \
    void set_ ## NAME(TYPE const &) { \
    }

template<typename INT_T>
inline std::ostream &operator<<(std::ostream &os, std::vector<INT_T> const &v) {
    for(auto &&c: v) {
        os << (char)((int)c > 31 && (int)c < 128 ? c : '.');
    }
    return os;
}

inline std::ostream &operator<<(std::ostream &os, std::vector<float> const &v) {
    std::string sep{};
    for(auto &&f: v) {
        os << sep << f;
        sep = ", ";
    }
    return os;
}

template<typename INT_T>
inline std::ostream &operator<<(std::ostream &os, std::deque<INT_T> const &v) {
   for(auto &&c: v) {
       os << (char)((int)c > 31 && (int)c < 128 ? c : '.');
   }
   return os;
}

template<typename INT_T, std::size_t N>
inline std::ostream &operator<<(std::ostream &os, std::array<INT_T, N> const &v) {
   for(auto &&c: v) {
       os << (char)((int)c > 31 && (int)c < 128 ? c : '.');
   }
   return os;
}

#define DEFINE_ARG_STRINGS_VECTOR(VAR_NAME, ARGC_NAME, ARGV_NAME) std::vector<std::string> VAR_NAME{ARGV_NAME, ARGV_NAME + ARGC_NAME}
#define DEFINE_ARG_WSTRINGS_VECTOR(VAR_NAME, ARGC_NAME, ARGV_NAME) std::vector<std::wstring> VAR_NAME{ARGV_NAME, ARGV_NAME + ARGC_NAME}
#define DEFINE_ARG_TSTRINGS_VECTOR(VAR_NAME, ARGC_NAME, ARGV_NAME) std::vector<lins::str_util::tstring> VAR_NAME{ARGV_NAME, ARGV_NAME + ARGC_NAME}
