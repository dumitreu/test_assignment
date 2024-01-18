#include "command_queue.hpp"
#include "math/math_util.hpp"

//#define TEST_LOG std::cout

namespace lins {

    ////////////////////////////// deferred_command_queue //////////////////////////////

    deferred_command_queue::deferred_command_queue(std::size_t thrd_cnt, std::size_t max_len):
    max_len_(lins::math::clamp<std::size_t>(max_len, 1, 1'000'000))
    {
        std::lock_guard pl(processors_mtp_);
        while(processors_.size() < thrd_cnt) {
            processors_.emplace_back(
                [this]() {
                    while(!termination()) {
                        std::function<void()> j;
                        if(fetch_job(j)) {
                            try { j(); } catch(...) {}
                        }
                    }
                }
            );
        }
    }

    deferred_command_queue::~deferred_command_queue() {
        terminate();
        {
            std::lock_guard el(evt_mtp_);
            evt_.notify_all();
        }
        {
            std::lock_guard pl(processors_mtp_);
            for(auto &&t: processors_) {
                if(t.joinable()) {
                    t.join();
                }
            }
            processors_.clear();
        }
    }

    void deferred_command_queue::add_job_timeout(std::function<void()> &&j, long double timeout) {
        while(jobs_size_approx() >= max_len_) {
            std::this_thread::sleep_for(std::chrono::milliseconds{1});
        }
        std::lock_guard el{evt_mtp_};
        jobs_[lins::timespec_wrapper::gmtnow().fseconds() + timeout] = std::move(j);
        approx_size_ = jobs_.size();
        evt_.notify_all();
    }

    void deferred_command_queue::add_job_timeout(std::function<void()> const &j, long double timeout) {
        while(jobs_size_approx() >= max_len_) {
            std::this_thread::sleep_for(std::chrono::milliseconds{1});
        }
        std::lock_guard el{evt_mtp_};
        jobs_[lins::timespec_wrapper::gmtnow().fseconds() + timeout] = j;
        approx_size_ = jobs_.size();
        evt_.notify_all();
    }

    void deferred_command_queue::add_job_gmt_fire_time(std::function<void()> &&j, lins::timespec_wrapper const &fire_time) {
        while(jobs_size_approx() >= max_len_) {
            std::this_thread::sleep_for(std::chrono::milliseconds{1});
        }
        std::lock_guard el{evt_mtp_};
        jobs_[fire_time.fseconds()] = std::move(j);
        approx_size_ = jobs_.size();
        evt_.notify_all();
    }

    void deferred_command_queue::add_job_gmt_fire_time(std::function<void()> const &j, lins::timespec_wrapper const &fire_time) {
        while(jobs_size_approx() >= max_len_) {
            std::this_thread::sleep_for(std::chrono::milliseconds{1});
        }
        std::lock_guard el{evt_mtp_};
        jobs_[fire_time.fseconds()] = j;
        approx_size_ = jobs_.size();
        evt_.notify_all();
    }

    bool deferred_command_queue::fetch_job(std::function<void()> &j) {
        std::unique_lock el(evt_mtp_);
        while(!termination()) {
            std::uint64_t time_to_wait{200};
            if(jobs_.size()) {
                if(0.005L + lins::timespec_wrapper::gmtnow().fseconds() - jobs_.begin()->first >= 0) {
                    j = std::move(jobs_.begin()->second);
                    jobs_.erase(jobs_.begin());
                    approx_size_ = jobs_.size();
                    return true;
                } else {
                    time_to_wait = jobs_.begin()->first * 1000 - lins::timespec_wrapper::gmtnow().milliseconds();
                }
            }
            if(!termination()) {
                evt_.wait_for(el, std::chrono::milliseconds{time_to_wait});
            }
        }
        return false;
    }

}

#ifdef TEST_LOG
#undef TEST_LOG
#endif
