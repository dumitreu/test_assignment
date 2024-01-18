#pragma once

#include "../../commondefs.hpp"
#ifdef PLATFORM_WINDOWS
#include <WinSock2.h>
#endif
#include "../../str_util.hpp"
#include <cstdint>
#include <ctime>
#include <time.h>

namespace {

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

    struct timezone {
        int  tz_minuteswest; /* minutes W of Greenwich */
        int  tz_dsttime;     /* type of dst correction */
    };

    int gettimeofday(struct timeval* tv, struct timezone* tz) {
        FILETIME ft;
        unsigned __int64 tmpres = 0;
        static int tzflag = 0;
        if (NULL != tv) {
            GetSystemTimeAsFileTime(&ft);
            tmpres |= ft.dwHighDateTime;
            tmpres <<= 32;
            tmpres |= ft.dwLowDateTime;
            tmpres /= 10;
            tmpres -= DELTA_EPOCH_IN_MICROSECS;
            tv->tv_sec = (long)(tmpres / 1000000UL);
            tv->tv_usec = (long)(tmpres % 1000000UL);
        }
        //if (NULL != tz) {
        //    if (!tzflag) {
        //        _tzset();
        //        tzflag++;
        //    }
        //    tz->tz_minuteswest = _timezone / 60;
        //    tz->tz_dsttime = _daylight;
        //}
        return 0;
    }

}

namespace lins {

    class timespec_wrapper {
    public:
        timespec_wrapper() : t_{ 0, 0 } {
        }

        timespec_wrapper(const timespec_wrapper& that) : t_{ that.t_ } {
        }

        timespec_wrapper& operator=(const timespec_wrapper& that) {
            if (&that != this) {
                t_ = that.t_;
            }
            return *this;
        }

        timespec_wrapper(time_t sec, long int nsec) : t_{ sec, nsec } {
        }

        timespec_wrapper(const struct timespec& that) : t_(that) {
        }

        explicit timespec_wrapper(std::string const& mysql_date) noexcept {
            from_iso_8601(mysql_date);
        }

        explicit timespec_wrapper(char const* mysql_date) {
            from_iso_8601(mysql_date);
        }

        explicit timespec_wrapper(std::uint64_t msec) {
            t_.tv_sec = msec / 1000L;
            t_.tv_nsec = (msec % 1000L) * 1000000L;
        }

        explicit timespec_wrapper(int msec) {
            t_.tv_sec = msec / 1000L;
            t_.tv_nsec = (msec % 1000L) * 1000000L;
        }

        timespec_wrapper(long double flt_val) {
            t_.tv_sec = static_cast<decltype(t_.tv_sec)>(flt_val);
            t_.tv_nsec = static_cast<decltype(t_.tv_nsec)>((flt_val - t_.tv_sec) * 1000000000L);
        }

        timespec_wrapper(const FILETIME &ft) {
            union { FILETIME t; std::int64_t i; } tmp;
            tmp.t = ft; tmp.i /= 10LL; tmp.i -= DELTA_EPOCH_IN_MICROSECS;
            t_.tv_sec = tmp.i / 1000000ULL;
            t_.tv_nsec = tmp.i % 1000000; t_.tv_nsec *= 1000;
        }

        std::int64_t seconds() const {
            return t_.tv_sec;
        }

        std::int64_t milliseconds() const {
            return ((std::int64_t)t_.tv_sec) * 1000LL + ((std::int64_t)t_.tv_nsec) / 1000000LL;
        }

        std::int64_t useconds() const {
            return ((std::int64_t)t_.tv_sec) * 1000000LL + ((std::int64_t)t_.tv_nsec) / 1000LL;
        }

        std::int64_t nseconds() const {
            return ((std::int64_t)t_.tv_sec) * 1000000000LL + ((std::int64_t)t_.tv_nsec);
        }

        long double fseconds() const {
            long double res = t_.tv_nsec;
            res /= 1e9L;
            res += t_.tv_sec;
            return res;
            //            return (long double)t_.tv_sec +
            //                   ((long double)t_.tv_nsec) / 1000000000.0L;
        }


        timespec_wrapper& from_iso_8601(std::string const& dts) {
            bool has_offs{ false };
            timespec_wrapper tw{ parse_iso_8601(dts, has_offs) };
            *this = tw;
            if (has_offs) {
                timespec_wrapper loff{ date_gmtoffset(tw) };
                *this += loff;
            }
            return *this;
        }

#ifdef PLATFORM_WINDOWS
#ifdef min
#undef min
#endif
#endif

        int year() const { struct tm* tmp{ ::localtime(&(t_.tv_sec)) }; return tmp->tm_year + 1900; }
        int month() const { struct tm* tmp{ ::localtime(&(t_.tv_sec)) }; return tmp->tm_mon + 1; }
        int day() const { struct tm* tmp{ ::localtime(&(t_.tv_sec)) }; return tmp->tm_mday; }
        int hour() const { struct tm* tmp{ ::localtime(&(t_.tv_sec)) }; return tmp->tm_hour; }
        int min() const { struct tm* tmp{ ::localtime(&(t_.tv_sec)) }; return tmp->tm_min; }
        int sec() const { struct tm* tmp{ ::localtime(&(t_.tv_sec)) }; return tmp->tm_sec; }


        static timespec_wrapper now() {
            timespec_wrapper result;
            struct timeval right_now;
            gettimeofday(&right_now, NULL);
            result.t_.tv_sec = right_now.tv_sec;
            result.t_.tv_nsec = right_now.tv_usec;
            result.t_.tv_nsec *= 1000UL;
            return result;

            //SYSTEMTIME st;
            //GetLocalTime(&st);
            //timespec_wrapper result;
            //SystemTimeToFileTime(&st, &result.t_);
            //return result;
        }

        static timespec_wrapper gmtnow() noexcept {
            timespec_wrapper result;
            struct timeval tv;
            gettimeofday(&tv, nullptr);
            time_t t{ tv.tv_sec };
            struct tm *ltm{ ::gmtime(&t) };
            tv.tv_sec = std::mktime(ltm);
            result.t_.tv_sec = tv.tv_sec;
            result.t_.tv_nsec = tv.tv_usec * 1000;
            return result;
        }

        static long double fnow() {
            struct timeval right_now;
            gettimeofday(&right_now, NULL);
            long double res{ static_cast<long double>(right_now.tv_usec) };
            res /= 1e6;
            res += right_now.tv_sec;
            return res;
        }

        timespec_wrapper& operator+=(const timespec_wrapper& rhs) {
            t_.tv_sec += rhs.t_.tv_sec + ((t_.tv_nsec + rhs.t_.tv_nsec) / 1000000000L > 0 ? 1 : 0);
            t_.tv_nsec = (t_.tv_nsec + rhs.t_.tv_nsec) % 1000000000L;
            return *this;
        }

        timespec_wrapper& operator-=(const timespec_wrapper& rhs) {
            if (rhs.t_.tv_nsec > t_.tv_nsec) {
                t_.tv_nsec += 1000000000L;
                t_.tv_sec -= 1;
            }
            t_.tv_sec -= rhs.t_.tv_sec;
            t_.tv_nsec -= rhs.t_.tv_nsec;
            return *this;
        }

        timespec_wrapper& operator*=(long double d) {
            long double t = fseconds();
            t *= d;
            *this = t;
            return *this;
        }

        timespec_wrapper& operator/=(long double d) {
            long double t = fseconds();
            t /= d;
            *this = t;
            return *this;
        }

        //        operator const struct timespec&() const {
        //            return t_;
        //        }

        //        operator struct timespec&() {
        //            return t_;
        //        }

        //        const struct timespec *operator&() const {
        //            return &t_;
        //        }

        //        struct timespec *operator&() {
        //            return &t_;
        //        }

        const struct timespec& ts() const {
            return t_;
        }

        struct timespec& ts() {
            return t_;
        }

        std::string to_mysql_datetime_str(std::size_t prec = 6) const {
            long double fsecs{ fseconds() };
            std::int64_t secs{ static_cast<std::int64_t>(fsecs) };
            long double subseconds{ fsecs - secs };

            std::stringstream ss;
            //struct tm* tmp{ std::localtime(&(t_.tv_sec)) };
            struct tm* tmp{ _localtime64(&(t_.tv_sec)) };
            ss << tmp->tm_year + 1900 << '-' << std::setfill('0')
                << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_mon + 1) << '-'
                << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_mday)
                << ' '
                << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_hour) << ':'
                << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_min) << ':'
                << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_sec);

            if (prec > 0) {
                if (prec > 6) { prec = 6; }
                for (std::size_t i = 0; i < prec; ++i) { subseconds *= 10.0L; }
                std::string sstr{ lins::str_util::itoa(static_cast<std::int64_t>(std::round(std::abs(subseconds)))) };
                while (sstr.size() < prec) { sstr = std::string{ "0" } + sstr; }
                ss << '.'; for (std::size_t i = 0; i < prec; ++i) { ss << sstr[i]; }
            }
            return ss.str();
        }

        std::string to_iso_8601_str(std::size_t prec = 6) const {
            long double fsecs{fseconds()};
            std::int64_t secs{static_cast<std::int64_t>(fsecs)};
            long double subseconds{fsecs - secs};

            std::stringstream ss;
            struct tm *tmp{::localtime(&(t_.tv_sec))};
            ss << tmp->tm_year + 1900 << '-' << std::setfill('0')
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_mon + 1) << '-'
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_mday)
               << 'T'
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_hour) << ':'
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_min) << ':'
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_sec);

            if(prec > 0) {
                if(prec > 6) { prec = 6; }
                for(std::size_t i = 0; i < prec; ++i) { subseconds *= 10.0L; }
                std::string sstr{lins::str_util::itoa(static_cast<std::int64_t>(std::round(std::abs(subseconds))))};
                while(sstr.size() < prec) { sstr = std::string{"0"} + sstr; }
                ss << '.'; for(std::size_t i = 0; i < prec; ++i) { ss << sstr[i]; }
            }

            std::int64_t time_offs{date_gmtoffset(*this).seconds()};
            std::int64_t hours_offs{time_offs / 3600};
            std::int64_t minutes_offs{(time_offs % 3600) / 60};
            if(hours_offs== 0) {
                ss << 'Z';
            } else {
                ss << (hours_offs < 0 ? "-" : "+")
                   << std::setfill('0') << std::setw(2) << std::abs(hours_offs) << ":"
                   << std::setfill('0') << std::setw(2) << std::abs(minutes_offs);
            }

            return ss.str();
        }


        static timespec_wrapper date_gmtoffset(timespec_wrapper const& d) noexcept {
            timespec_wrapper result;
            struct timeval tv {0};
            tv.tv_sec = d.t_.tv_sec;
            time_t t{ tv.tv_sec };
            struct tm *ltm{::localtime(&t)};
            int lh{ltm->tm_hour};
            int lm{ltm->tm_min};
            struct tm *gtm{::gmtime(&t)};
            result.t_.tv_sec = (lh - gtm->tm_hour) * 3600 + (lm - gtm->tm_min) * 60;
            result.t_.tv_nsec = 0;
            return result;
        }

        friend timespec_wrapper operator+(const timespec_wrapper& l,
            const timespec_wrapper& r) {
            timespec_wrapper result(l); result += r; return result;
        }

        friend timespec_wrapper operator-(const timespec_wrapper& l,
            const timespec_wrapper& r) {
            timespec_wrapper result(l); result -= r; return result;
        }

        friend timespec_wrapper operator*(const timespec_wrapper& l,
            double r) {
            timespec_wrapper result(l); result *= r; return result;
        }

        friend timespec_wrapper operator/(const timespec_wrapper& l,
            double r) {
            timespec_wrapper result(l); result /= r; return result;
        }

        friend bool operator<(const timespec_wrapper& l, const timespec_wrapper& r) {
            return (l.t_.tv_sec == r.t_.tv_sec && l.t_.tv_nsec < r.t_.tv_nsec) || l.t_.tv_sec < r.t_.tv_sec;
        }

        friend bool operator>(const timespec_wrapper& l, const timespec_wrapper& r) {
            return (l.t_.tv_sec == r.t_.tv_sec && l.t_.tv_nsec > r.t_.tv_nsec) || l.t_.tv_sec > r.t_.tv_sec;
        }

        friend bool operator==(const timespec_wrapper& l, const timespec_wrapper& r) {
            return l.t_.tv_sec == r.t_.tv_sec && l.t_.tv_nsec == r.t_.tv_nsec;
        }

        friend bool operator<=(const timespec_wrapper& l, const timespec_wrapper& r) {
            return (l.t_.tv_sec == r.t_.tv_sec && l.t_.tv_nsec <= r.t_.tv_nsec) || (l.t_.tv_sec < r.t_.tv_sec);
        }

        friend bool operator>=(const timespec_wrapper& l, const timespec_wrapper& r) {
            return (l.t_.tv_sec == r.t_.tv_sec && l.t_.tv_nsec >= r.t_.tv_nsec) || (l.t_.tv_sec > r.t_.tv_sec);
        }

        friend bool operator!=(const timespec_wrapper& l, const timespec_wrapper& r) {
            return l.t_.tv_nsec != r.t_.tv_nsec || l.t_.tv_sec != r.t_.tv_sec;
        }

    private:
        static timespec_wrapper parse_iso_8601(std::string const& dts, bool& gmt_offset_is_given) {
            timespec_wrapper result{};
            try {
                if (dts.size() > 0) {
                    enum class states { ye, mo, da, ho, mi, se, sf, oh, om, fi };
                    std::array<std::int64_t, (std::size_t)states::fi> brdn{};
                    int sec_frac_nums{ 0 };
                    states state{ states::ye };
                    std::string buf{};
                    bool neg{ false };
                    long double offs_sign{ 1.0L };
                    gmt_offset_is_given = false;
                    for (std::size_t cno{ 0 }; cno < dts.size() && state >= states::ye && state < states::fi; ++cno) {
                        char currc{ dts[cno] };
                        if (!std::isdigit(currc)) {
                            if (state == states::ye && buf.empty() && (currc == '-' || currc == '+')) {
                                neg = currc == '-';
                            }
                            if (!buf.empty()) {
                                brdn[(int)state] = lins::str_util::atoi(std::string{ buf });
                                buf.clear();
                                state = (states)((int)state + 1);
                            }
                            if ((int)state >= (int)states::ho && (int)state <= (int)states::sf) {
                                if ((currc == '-' || currc == '+') || std::toupper(currc) == 'Z') {
                                    state = states::oh;
                                }
                            }
                            if (state == states::oh) {
                                if (buf.empty()) {
                                    if (currc == '-') {
                                        offs_sign = -1.0L;
                                    }
                                    if (std::toupper(currc) == 'Z') {
                                        gmt_offset_is_given = true;
                                        state = states::fi;
                                    }
                                }
                            }
                        }
                        else {
                            buf += currc;
                            if (state == states::sf) { ++sec_frac_nums; }
                            if (state == states::oh) {
                                gmt_offset_is_given = true;
                                if (buf.size() == 2 && cno + 1 < dts.size() && std::isdigit(dts[cno + 1])) {
                                    brdn[(int)state] = lins::str_util::atoi(std::string{ buf });
                                    buf.clear();
                                    state = (states)((int)state + 1);
                                }
                            }
                            if (state == states::om) {
                                gmt_offset_is_given = true;
                                if (buf.size() == 2 && cno + 1 < dts.size() && std::isdigit(dts[cno + 1])) {
                                    brdn[(int)state] = lins::str_util::atoi(std::string{ buf });
                                    buf.clear();
                                    state = (states)((int)state + 1);
                                }
                            }
                            if (cno + 1 == dts.size() && !buf.empty()) {
                                std::int64_t bval{ lins::str_util::atoi(std::string{buf}) };
                                brdn[(int)state] = bval;
                            }
                        }
                    }
                    struct tm tm;
                    tm.tm_sec = brdn[(int)states::se];
                    tm.tm_min = brdn[(int)states::mi];
                    tm.tm_hour = brdn[(int)states::ho];
                    tm.tm_mday = brdn[(int)states::da];
                    tm.tm_mon = brdn[(int)states::mo] - 1;
                    tm.tm_year = (neg ? -brdn[(int)states::ye] : brdn[(int)states::ye]) - 1900;
                    tm.tm_isdst = -1;
                    long double res{ static_cast<long double>(std::mktime(&tm)) };
                    if (brdn[(int)states::sf] > 0) {
                        long double fssdiv{ 1.0L };
                        for (int i{}; i < sec_frac_nums; ++i) { fssdiv *= 10.0L; }
                        long double fss{ (long double)brdn[(int)states::sf] / fssdiv };
                        res += res < 0 ? -(1 - fss) : fss;
                    }
                    result.t_.tv_sec = res;
                    result.t_.tv_nsec = (std::abs(res) - std::abs(result.t_.tv_sec)) * 1'000'000'000LL;

                    if (brdn[(int)states::oh] || brdn[(int)states::om]) {
                        timespec_wrapper off{ (brdn[(int)states::oh] * 3600.0L + brdn[(int)states::om] * 60.0L) * offs_sign };
                        result = result - off;
                    }
                }
                else {
                    result.t_.tv_sec = 0LL;
                    result.t_.tv_nsec = 0;
                }
            }
            catch (...) {
                result.t_.tv_sec = 0LL;
                result.t_.tv_nsec = 0;
            }
            return result;
        }

    private:
        struct timespec t_;
    };

    static const timespec_wrapper eternity{ static_cast<time_t>(0x7fff'ffff'ffff'ffffLL), static_cast<long>(0) };

}


