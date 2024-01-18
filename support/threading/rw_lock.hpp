#pragma once

#include "../commondefs.hpp"
#include "../timespec_wrapper.hpp"
#include "../sys_util.hpp"
#include "../file_util.hpp"

namespace lins {

    class rw_lock final {
    public:
        rw_lock(bool prefer_writers = true) noexcept : prefer_writers_{prefer_writers} {
        }

        ~rw_lock() = default;

        rw_lock(const rw_lock &) = delete;

        rw_lock& operator=(const rw_lock &) = delete;

        rw_lock(rw_lock &&that) = delete;

        rw_lock &operator=(rw_lock &&that) = delete;

        bool lock(timespec_wrapper const &timeout = eternity, bool w = true) noexcept {
            if(w) {
                return lock_for_write(timeout);
            } else {
                return lock_for_read(timeout);
            }
        }

        bool unlock(bool w) noexcept {
            if(w) {
                return write_unlock();
            } else {
                return read_unlock();
            }
        }

        bool try_lock(bool w = true) noexcept {
            if(w) {
                return try_lock_for_write();
            } else {
                return try_lock_for_read();
            }
        }

        bool upgrade() noexcept {
#ifdef EXIT_ON_DEADSPIN
            lins::shut_on_destroy scf{[this]() { spins_count_ = 0; }};
#endif
            int curr{current_.load()};
            if(curr <= 0) {
                return false;
            }
            pending_upgraders_.fetch_add(1);
            lins::shut_on_destroy up{[this]() { pending_upgraders_.fetch_sub(1); }};
            int one_wanted{1};
            while(!current_.compare_exchange_strong(one_wanted, -1)) {
                one_wanted = 1;
                maybe_wait();
            }
            return true;
        }

        bool downgrade() noexcept {
            if(current_.load() == -1) {
                current_.store(1);
                return true;
            }
            return false;
        }

        void reset(bool prefer_writers = true) noexcept {
            current_ = 0;
            pending_upgraders_ = 0;
            pending_readers_ = 0;
            pending_writers_ = 0;
#ifdef EXIT_ON_DEADSPIN
            spins_count_ = 0;
#endif
            prefer_writers_ = prefer_writers;
        }

    private:
        bool lock_for_write(timespec_wrapper const &timeout) noexcept {
#ifdef EXIT_ON_DEADSPIN
            lins::shut_on_destroy scf{[this]() {
                    spins_count_ = 0;
                }
            };
#endif
            pending_writers_.fetch_add(1);
            lins::shut_on_destroy up{[this]() { pending_writers_.fetch_sub(1); }};
            bool frvr{timeout == eternity};
            timespec_wrapper later{frvr ? eternity : timespec_wrapper::now() + timeout};
            while(frvr || timespec_wrapper::now() < later) {
                if(pending_upgraders_.load() == 0 && (prefer_writers_ || pending_readers_.load() == 0)) {
                    int zero{0};
                    if(current_.compare_exchange_strong(zero, -1)) {
                        return true;
                    }
                }
                maybe_wait();
            }
            return false;
        }

        bool lock_for_read(timespec_wrapper const &timeout) noexcept {
#ifdef EXIT_ON_DEADSPIN
            lins::shut_on_destroy scf{[this]() {
                    spins_count_ = 0;
                }
            };
#endif
            pending_readers_.fetch_add(1);
            lins::shut_on_destroy up{[this]() { pending_readers_.fetch_sub(1); }};
            bool frvr{timeout == eternity};
            timespec_wrapper later{frvr ? eternity : timespec_wrapper::now() + timeout};
            while(frvr || timespec_wrapper::now() < later) {
                if(pending_upgraders_.load() == 0 && (!prefer_writers_ || pending_writers_.load() == 0)) {
                    int curr{current_.load()};
                    if(curr >= 0 && current_.compare_exchange_strong(curr, curr + 1)) {
                        return true;
                    }
                }
                maybe_wait();
            }
            return false;
        }

        bool try_lock_for_write() noexcept {
            pending_writers_.fetch_add(1);
            lins::shut_on_destroy up{[this]() { pending_writers_.fetch_sub(1); }};
            bool res{false};
            if(pending_upgraders_.load() == 0 && (prefer_writers_ || pending_readers_.load() == 0)) {
                int zero{0};
                if(current_.compare_exchange_strong(zero, -1)) {
                    res = true;
                }
            }
            return res;
        }

        bool try_lock_for_read() noexcept {
            pending_readers_.fetch_add(1);
            lins::shut_on_destroy up{[this]() { pending_readers_.fetch_sub(1); }};
            bool res{false};
            if(pending_upgraders_.load() == 0 && (!prefer_writers_ || pending_writers_.load() == 0)) {
                int curr{current_.load()};
                if(curr >= 0 && current_.compare_exchange_strong(curr, curr + 1)) {
                    res = true;
                }
            }
            return res;
        }

        bool read_unlock() noexcept {
            int curr{current_.load()};
            if(curr <= 0) {
                return false;
            }
            current_.fetch_sub(1);
            return true;
        }

        bool write_unlock() noexcept {
            int curr{current_.load()};
            if(curr == -1) {
                current_.store(0);
                return true;
            }
            return false;
        }

    private:
        void maybe_wait() {
#ifdef EXIT_ON_DEADSPIN
    #ifdef ATOMIC_SM_SLEEP
            std::this_thread::sleep_for(std::chrono::microseconds(500));
            ++spins_count_;
            if(spins_count_ > 100'000) {
                std::string bts{lins::sys_util::backtrace()};
                lins::file_util::save_to_file("/tmp/doberchat.backtrace", bts);
                exit(1);
            }
    #else
            ++spins_count_;
            if(spins_count_ > 1'000'000'000) {
                std::string bts{lins::sys_util::backtrace()};
                lins::file_util::save_to_file("/tmp/doberchat.backtrace", bts);
                exit(1);
            }
    #endif
#else
    #ifdef ATOMIC_SM_SLEEP
            std::this_thread::sleep_for(std::chrono::microseconds{500});
    #endif
#endif
        }

    private:
        std::atomic<int> current_{0};
        std::atomic<int> pending_upgraders_{0};
        std::atomic<int> pending_readers_{0};
        std::atomic<int> pending_writers_{0};
#ifdef EXIT_ON_DEADSPIN
        std::atomic<std::int64_t> spins_count_{0};
#endif
        bool prefer_writers_{true};
    };

}
