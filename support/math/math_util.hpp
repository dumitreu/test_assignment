#pragma once

#include "../commondefs.hpp"

namespace lins::math {

    static float df(std::function<float(float)> const &f, float x, float dx = 1e-4) {
        float f0{f(x)};
        float f1{f(x + dx)};
        return (f1 - f0) / dx;
    }
    template<typename FUNC_T>
    float df(FUNC_T const &f, float x, float dx = 1e-4) {
        float f0{f(x)};
        float f1{f(x + dx)};
        return (f1 - f0) / dx;
    }

    static double df(std::function<double(double)> const &f, double x, double dx = 1e-6) {
        double f0{f(x)};
        double f1{f(x + dx)};
        return (f1 - f0) / dx;
    }
    template<typename FUNC_T>
    double df(FUNC_T const &f, double x, double dx = 1e-6) {
        double f0{f(x)};
        double f1{f(x + dx)};
        return (f1 - f0) / dx;
    }

    static long double df(std::function<long double(long double)> const &f, long double x, long double dx = 1e-6) {
        long double f0{f(x)};
        long double f1{f(x + dx)};
        return (f1 - f0) / dx;
    }
    template<typename FUNC_T>
    long double df(FUNC_T const &f, long double x, long double dx = 1e-6) {
        long double f0{f(x)};
        long double f1{f(x + dx)};
        return (f1 - f0) / dx;
    }


    template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    T sigmoid(T x) { return 1 / (1 + std::exp(-x)); }

    template<typename T>
    T sigmoid(T x, T xshift = 0, T yshift = 0, T slope = 1) {
        return 1 / (1 + std::exp(-(x - xshift) * slope)) + yshift;
    }

#ifdef PLATFORM_WINDOWS
#if defined min
#undef min
#endif
#if defined max
#undef max
#endif
#endif

    template<typename T>
    T min(T v1, T v2) { return v1 < v2 ? v1 : v2; }

    template<typename T>
    T max(T v1, T v2) { return v2 < v1 ? v1 : v2; }

    template<typename T>
    T clamp(T const &value, T const &low, T const &hi) {
        if(value < low) { return low; }
        if(value > hi) { return hi; }
        return value;
    }

    template<typename T>
    T sign(T v) { return v < 0 ? -1 : 1; }

    template<typename T>
    T abs(T v) { return v < 0 ? -v : v; }

    template<typename T>
    T sqr(T v) { return v * v; }

    template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    T deg2rad(T d) { return d * M_PIl / 180.0L; }

    template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    T rad2deg(T r) { return r * 180.0L / M_PIl; }


    template<typename T>
    void normalize(T &v) {
        if(!v.empty()) {
            auto vmax{v[0]};
            for(auto x: v) { vmax = vmax < x ? x : vmax; }
            if(vmax > 0) { for(auto &x: v) { x /= vmax; } }
        }
    }

    template<typename FP_T>
    FP_T gaussian(FP_T x, FP_T mat_exp = 0, FP_T std_dev = 1) {
        return std::exp(-sqr(x - mat_exp) / (2 * sqr(std_dev))) / (std_dev * std::sqrt(2 * M_PI));
    }

    template<typename FP_T>
    class norm_dist_rng {
    public:
        norm_dist_rng(FP_T mean, FP_T dis): mean_{mean}, dis_{dis}, nrd_{mean, std::abs(dis) / 6} {}
        FP_T gen() { return nrd_(rd_); }
        FP_T operator()() { return gen(); }

    private:
        FP_T mean_;
        FP_T dis_;
        std::random_device rd_{};
        std::normal_distribution<FP_T> nrd_;
    };

}
