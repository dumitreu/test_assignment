#pragma once

#include "commondefs.hpp"

#define INCLUDED_FROM_THREADING_MAIN

#if defined(PLATFORM_WINDOWS)
#if defined(ENVIRONMENT64)
    #include "threading/windows/mt.hpp"
#elif defined(ENVIRONMENT32)
    #include "threading/windows/mt.hpp"
#endif
#elif defined(PLATFORM_APPLE)
    #include "threading/pthreads/mt.hpp"
#elif defined(PLATFORM_ANDROID)
    #include "threading/stl/mt.hpp"
#elif defined(PLATFORM_LINUX)
    #include "threading/pthreads/mt.hpp"
#elif defined(PLATFORM_UNIXISH)
    #include "threading/pthreads/mt.hpp"
#elif defined(PLATFORM_POSIX)
    #include "threading/pthreads/mt.hpp"
#endif

#undef INCLUDED_FROM_THREADING_MAIN

#include "threading/spin_lock.hpp"
#include "threading/rw_lock.hpp"
#include "threading/scoped_locker.hpp"

namespace lins {

#ifdef DBG_APP_MT_LOCKING
    class dbg_mutex {
    public:
        dbg_mutex() = default;
        dbg_mutex(dbg_mutex &&) = delete;
        dbg_mutex(dbg_mutex const &) = delete;
        dbg_mutex &operator=(dbg_mutex &&) = delete;
        dbg_mutex &operator=(dbg_mutex const &) = delete;
        ~dbg_mutex() = default;

        void lock() {
            while(!try_lock()) {
                if(try_lock_count_.fetch_add(1) > 600'000) {
                    std::string bts{lins::sys_util::backtrace()};
                    std::this_thread::get_id();
                    lins::file_util::save_to_file(std::string{"/tmp/deadlock."} + lins::timespec_wrapper::now().to_mysql_datetime_str(), bts);
                    //exit(1);
                    char *crash_buff_ptr{(char *)0xdeadbeef};
                    *crash_buff_ptr = 42;
                }
                std::this_thread::sleep_for(std::chrono::microseconds{100});
            }
            try_lock_count_.store(0);
        }

        bool try_lock() noexcept {
            return m_.try_lock();
        }

        void unlock() { m_.unlock(); }

        std::mutex::native_handle_type native_handle() {
            return m_.native_handle();
        }

    private:
        std::atomic<std::uint64_t> try_lock_count_{0ULL};
        std::mutex m_{};
    };

    class dbg_shared_mutex {
    public:
        dbg_shared_mutex() = default;
        dbg_shared_mutex(dbg_shared_mutex &&) = delete;
        dbg_shared_mutex(dbg_shared_mutex const &) = delete;
        dbg_shared_mutex &operator=(dbg_shared_mutex &&) = delete;
        dbg_shared_mutex &operator=(dbg_shared_mutex const &) = delete;
        ~dbg_shared_mutex() = default;

        void lock() {
            while(!try_lock()) {
                if(try_lock_count_.fetch_add(1) > 600'000) {
                    std::string bts{lins::sys_util::backtrace()};
                    lins::file_util::save_to_file(std::string{"/tmp/deadlock."} + lins::timespec_wrapper::now().to_mysql_datetime_str(), bts);
//                    exit(1);
                    char *crash_buff_ptr{(char *)0xdeadbeef};
                    *crash_buff_ptr = 42;
                }
                std::this_thread::sleep_for(std::chrono::microseconds{100});
            }
            try_lock_count_.store(0);
        }

        bool try_lock() {
            return m_.try_lock();
        }

        void unlock() { m_.unlock(); }


        // Shared ownership

        void lock_shared() {
            while(!try_lock_shared()) {
                if(try_lock_count_.fetch_add(1) > 600'000) {
                    std::string bts{lins::sys_util::backtrace()};
                    lins::file_util::save_to_file(std::string{"/tmp/deadlock."} + lins::timespec_wrapper::now().to_mysql_datetime_str(), bts);
//                    exit(1);
                    char *crash_buff_ptr{(char *)0xdeadbeef};
                    *crash_buff_ptr = 42;
                }
                std::this_thread::sleep_for(std::chrono::microseconds{100});
            }
            try_lock_count_.store(0);
        }

        bool try_lock_shared() {
            return m_.try_lock_shared();
        }

        void unlock_shared() {
            m_.unlock_shared();
        }

    private:
        std::atomic<std::uint64_t> try_lock_count_{0ULL};
        std::shared_mutex m_{};
    };

#else
    using dbg_mutex = std::mutex;
    using dbg_shared_mutex = std::shared_mutex;
#endif


    class nonblocking_mutex {
    public:
        nonblocking_mutex() = default;
        nonblocking_mutex(nonblocking_mutex &&) = default;
        nonblocking_mutex(nonblocking_mutex const &) = delete;
        nonblocking_mutex &operator=(nonblocking_mutex &&) = default;
        nonblocking_mutex &operator=(nonblocking_mutex const &) = delete;
        ~nonblocking_mutex() = default;

        void lock() {}

        bool try_lock() noexcept {
            return true;
        }

        void unlock() {}

        std::mutex::native_handle_type native_handle() {
            return std::mutex::native_handle_type{};
        }
    };

    class nonblocking_shared_mutex {
    public:
        nonblocking_shared_mutex() = default;
        nonblocking_shared_mutex(nonblocking_shared_mutex &&) = delete;
        nonblocking_shared_mutex(nonblocking_shared_mutex const &) = delete;
        nonblocking_shared_mutex &operator=(nonblocking_shared_mutex &&) = delete;
        nonblocking_shared_mutex &operator=(nonblocking_shared_mutex const &) = delete;
        ~nonblocking_shared_mutex() = default;

        void lock() {
        }

        bool try_lock() {
            return true;
        }

        void unlock() {}


        // Shared ownership

        void lock_shared() {}

        bool try_lock_shared() {
            return true;
        }

        void unlock_shared() {
        }

    };


    static /*(useconds)*/inline std::uint64_t curr_timestamp() noexcept {
        return static_cast<std::uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count()) / 1'000ULL;
    }

    static /*(seconds)*/inline long double curr_timestamp_seconds() noexcept {
        return static_cast<long double>(std::chrono::steady_clock::now().time_since_epoch().count()) / 1'000'000'000.0L;
    }

    template<typename T>
    class thread_safe_wrapper_stl {
    public:
        thread_safe_wrapper_stl() {}

        template<typename...Args>
        thread_safe_wrapper_stl(Args &&...a): v_{std::forward<Args>(a)...} {}

        thread_safe_wrapper_stl(const thread_safe_wrapper_stl &that) {
            std::shared_lock that_lck{that.m_};
            v_ = that.v_;
        }

        thread_safe_wrapper_stl &operator=(const thread_safe_wrapper_stl &that) {
            if(&that != this) {
                std::shared_lock that_lck{that.m_};
                std::unique_lock this_lck{m_};
                v_ = that.v_;
            }
            return *this;
        }

        thread_safe_wrapper_stl(thread_safe_wrapper_stl &&that) {
            std::unique_lock that_lck{that.m_};
            v_ = std::move(that.v_);
        }

        thread_safe_wrapper_stl &operator=(thread_safe_wrapper_stl &&that) {
            if(&that != this) {
                std::unique_lock that_lck{that.m_};
                std::unique_lock this_lck{m_};
                v_ = std::move(that.v_);
            }
            return *this;
        }

        thread_safe_wrapper_stl &operator=(T const &v) {
            std::unique_lock this_lck{m_};
            v_ = v;
            return *this;
        }

        thread_safe_wrapper_stl &operator=(T &&v) {
            std::unique_lock this_lck{m_};
            v_ = std::move(v);
            return *this;
        }

        ~thread_safe_wrapper_stl() = default;

        class proxy {
            friend class thread_safe_wrapper_stl;
            proxy(thread_safe_wrapper_stl &w, bool lock_kind): w_{w} {
                if(lock_kind) {
                    w_lckr_ = std::make_unique<std::unique_lock<dbg_shared_mutex>>(w_.m_);
                } else {
                    r_lckr_ = std::make_unique<std::shared_lock<dbg_shared_mutex>>(w_.m_);
                }
            }

        public:
            proxy(const proxy&) = delete;
            proxy &operator=(const proxy&) = delete;
            proxy(proxy&& that) = delete;
            proxy &operator=(proxy&& that) = delete;
            ~proxy() = default;
            T *operator->() { return &(w_.v_); }
            const T *operator->() const { return &(w_.v_); }
            proxy &operator=(T const &v) { w_.v_ = v; return *this; }
            operator T const &() const { return w_.v_; }
            operator T &() { return w_.v_; }
            T const &val() const { return w_.v_; }
            T &val() { return w_.v_; }
            T &operator*() { return w_.v_; }
            T const &operator*() const { return w_.v_; }

        private:
            thread_safe_wrapper_stl &w_;
            std::unique_ptr<std::unique_lock<dbg_shared_mutex>> w_lckr_{};
            std::unique_ptr<std::shared_lock<dbg_shared_mutex>> r_lckr_{};
        };

        class const_proxy {
            friend class thread_safe_wrapper_stl;
            const_proxy(thread_safe_wrapper_stl const &w, bool lock_kind): w_{w} {
                if(lock_kind) {
                    w_lckr_ = std::make_unique<std::unique_lock<dbg_shared_mutex>>(w_.m_);
                } else {
                    r_lckr_ = std::make_unique<std::shared_lock<dbg_shared_mutex>>(w_.m_);
                }
            }

        public:
            const_proxy(const const_proxy &) = delete;
            const_proxy &operator=(const const_proxy &) = delete;
            const_proxy(const_proxy &&that) = delete;
            const_proxy &operator=(const_proxy &&that) = delete;
            ~const_proxy() = default;
            const T *operator->() const { return &(w_.v_); }
            operator T const &() const { return w_.v_; }
            T const &val() const { return w_.v_; }
            T const &operator*() const { return w_.v_; }

        private:
            thread_safe_wrapper_stl const &w_;
            std::unique_ptr<std::unique_lock<dbg_shared_mutex>> w_lckr_{};
            std::unique_ptr<std::shared_lock<dbg_shared_mutex>> r_lckr_{};
        };

        proxy operator--(int) { return proxy{*this, lock_kind_}; }
        const_proxy operator--(int) const { return const_proxy{*this, false}; }
        proxy operator++(int) { return proxy{*this, true}; }
        T *operator->() { return &v_; }
        const T *operator->() const { return &v_; }
        proxy operator*() { return proxy{*this, lock_kind_}; }
        const_proxy operator*() const { return const_proxy{*this, false}; }
        proxy locked_object() { return proxy{*this, lock_kind_}; }
        const_proxy locked_object() const { return const_proxy{*this, false}; }
        T &ref() { return v_; }
        T const &ref() const { return v_; }

        void set_one_shot_lock_kind(bool kind) { lock_kind_ = kind; }
        bool one_shot_lock_kind() const { return lock_kind_; }

        // locking interface
        void lock() { m_.lock(); }
        bool try_lock() { return m_.try_lock(); }
        void unlock() { m_.unlock(); }
        void lock_shared() { m_.lock_shared(); }
        bool try_lock_shared() { return m_.try_lock_shared(); }
        void unlock_shared() { m_.unlock_shared(); }

    private:
        T v_{};
        mutable dbg_shared_mutex m_{};
        bool lock_kind_{true};
    };


    template<typename T>
    class thread_safe_wrapper_lins {
    public:
        thread_safe_wrapper_lins() {}

        template<typename ...Args>
        thread_safe_wrapper_lins(Args &&...a) : v_{std::forward<Args>(a)...} {}

        thread_safe_wrapper_lins(const thread_safe_wrapper_lins &that) {
            lins::scoped_locker that_lck{that.m_};
            v_ = that.v_;
        }

        thread_safe_wrapper_lins &operator=(const thread_safe_wrapper_lins &that) {
            if(&that != this) {
                lins::scoped_locker that_lck{that.m_};
                lins::scoped_locker this_lck{m_};
                v_ = that.v_;
            }
            return *this;
        }

        thread_safe_wrapper_lins(thread_safe_wrapper_lins &&that) {
            lins::scoped_locker that_lck{that.m_};
            v_ = std::move(that.v_);
        }

        thread_safe_wrapper_lins &operator=(thread_safe_wrapper_lins &&that) {
            if(&that != this) {
                lins::scoped_locker that_lck{that.m_};
                lins::scoped_locker this_lck{m_};
                v_ = std::move(that.v_);
            }
            return *this;
        }

        thread_safe_wrapper_lins &operator=(T const &v) {
            lins::scoped_locker this_lck{m_, true};
            v_ = v;
            return *this;
        }

        thread_safe_wrapper_lins &operator=(T &&v) {
            lins::scoped_locker this_lck{m_, true};
            v_ = std::move(v);
            return *this;
        }

        ~thread_safe_wrapper_lins() = default;

        class proxy {
            friend class thread_safe_wrapper_lins;
            proxy(thread_safe_wrapper_lins &w, bool lock_kind): w_{w}, lckr_{w_.m_, lock_kind} {}

        public:
            proxy(const proxy&) = delete;
            proxy &operator=(const proxy&) = delete;
            proxy(proxy&& that) = delete;
            proxy &operator=(proxy&& that) = delete;
            ~proxy() { lckr_.unlock(); }
            T *operator->() { return &(w_.v_); }
            const T *operator->() const { return &(w_.v_); }
            proxy &operator=(T const &v) { w_.v_ = v; return *this; }
            operator T const &() const { return w_.v_; }
            operator T &() { return w_.v_; }
            T const &val() const { return w_.v_; }
            T &val() { return w_.v_; }
            T &operator*() { return w_.v_; }
            T const &operator*() const { return w_.v_; }

        private:
            thread_safe_wrapper_lins &w_;
            lins::scoped_locker<lins::rw_lock> lckr_;
        };

        class const_proxy {
            friend class thread_safe_wrapper_lins;
            const_proxy(thread_safe_wrapper_lins const &w, bool lock_kind): w_{w}, lckr_{w_.m_, lock_kind} {}

        public:
            const_proxy(const const_proxy &) = delete;
            const_proxy &operator=(const const_proxy &) = delete;
            const_proxy(const_proxy &&that) = delete;
            const_proxy &operator=(const_proxy &&that) = delete;
            ~const_proxy() { lckr_.unlock(); }
            const T *operator->() const { return &(w_.v_); }
            operator T const &() const { return w_.v_; }
            T const &val() const { return w_.v_; }
            T const &operator*() const { return w_.v_; }

        private:
            thread_safe_wrapper_lins const &w_;
            lins::scoped_locker<lins::rw_lock> lckr_;
        };

        proxy operator--(int) { return proxy{*this, lock_kind_}; }
        const_proxy operator--(int) const { return const_proxy{*this, false}; }
        proxy operator++(int) { return proxy{*this, true}; }
        T *operator->() { return &v_; }
        const T *operator->() const { return &v_; }
        proxy operator*() { return proxy{*this, lock_kind_}; }
        const_proxy operator*() const { return const_proxy{*this, false}; }
        proxy locked_object() { return proxy{*this, lock_kind_}; }
        const_proxy locked_object() const { return const_proxy{*this, false}; }
        T &ref() { return v_; }
        T const &ref() const { return v_; }

        void set_one_shot_lock_kind(bool kind) { lock_kind_ = kind; }
        bool one_shot_lock_kind() const { return lock_kind_; }

        // locking interface
        bool lock(timespec_wrapper const &timeout = eternity, bool kind = true) { return m_.lock(timeout, kind); }
        bool try_lock(bool kind = true) { return m_.try_lock(kind); }
        bool unlock(bool kind) { return m_.unlock(kind); }
        bool upgrade() { return m_.upgrade(); }
        bool downgrade() { return m_.downgrade(); }

    private:
        T v_{};
        mutable lins::rw_lock m_{};
        bool lock_kind_{true};
    };

}
