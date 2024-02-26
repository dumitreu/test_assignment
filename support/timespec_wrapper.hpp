#pragma once

#include "commondefs.hpp"

#if defined(PLATFORM_WINDOWS)
    #include "time/windows/timespec_wrapper.hpp"
#elif defined(PLATFORM_APPLE)
    #include "time/apple/timespec_wrapper.hpp"
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
    #include "time/posix/timespec_wrapper.hpp"
#elif defined(PLATFORM_UNIX)
    #include "time/posix/timespec_wrapper.hpp"
#elif defined(PLATFORM_POSIX)
    #include "time/posix/timespec_wrapper.hpp"
#endif

inline std::ostream &operator<<(std::ostream &os, lins::timespec_wrapper const &ts) {
    os << ts.to_mysql_datetime_str();
    return os;
}
