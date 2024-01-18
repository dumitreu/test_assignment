#pragma once

#include "../commondefs.hpp"
#include "../timespec_wrapper.hpp"

// 1 = nanosleep, 0 = no sleep
#define ATOMIC_SM_SLEEP 1

#if ATOMIC_SM_SLEEP
#define WAIT_LOCK std::this_thread::sleep_for(std::chrono::nanoseconds(100))
#else
#define WAIT_LOCK
#endif

namespace lins {

    class spin_lock {
    public:
        spin_lock(): latch_(false) {
        }

        bool lock(timespec_wrapper const &timeout = eternity, bool = true) {
            bool frvr{timeout == eternity};
            timespec_wrapper later{frvr ? eternity : timespec_wrapper::now() + timeout};
            for(bool ul = false; !latch_.compare_exchange_strong(ul, true); ul = false) {
                if(frvr || timespec_wrapper::now() < later) {
                    WAIT_LOCK;
                } else {
                    return false;
                }
            }
            return true;
        }

        bool unlock(bool = true) {
            bool curr{true};
            return curr ? latch_.compare_exchange_strong(curr, false) : false;
        }

        bool upgrade() {
            return false;
        }

        bool downgrade() {
            return false;
        }

        bool try_lock(bool = true) noexcept {
            bool ul = false;
            return latch_.compare_exchange_strong(ul, true);
        }

    private:
        std::atomic_bool latch_;
    };

}

#undef ATOMIC_SM_SLEEP
#undef WAIT_LOCK
