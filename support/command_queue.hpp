#pragma once

#include "commondefs.hpp"
#include "threading.hpp"
#include "terminable.hpp"
#include "containers/ring_buffer.hpp"

namespace lins {

    class command_queue final: public lins::terminable {
    public:
        command_queue(
            std::size_t max_thrd_cnt = std::thread::hardware_concurrency() * 2,
            std::size_t max_len = 10'000'000
        ):
            max_threads_{max_thrd_cnt},
            max_queue_len_{std::max<std::size_t>(max_len, 1)}
        {
            arbiter_thread_ = std::thread{
                [this]() {

                    std::uint64_t cycle{};
                    lins::ring_buffer<std::pair<std::uint64_t, std::uint64_t>, 16> jobs_enqueuing_timings{};

                    long double last_thread_spawning_time{0};
                    long double last_thread_killing_time{0};

                    while(!termination()) {
                        ++cycle;
                        (void)cycle;

                        try {
                            // setup queue dynamics
                            std::uint64_t jobs_in_q{jobs_size_approx()};

                            // average queue dynamics
                            jobs_enqueuing_timings.emplace_back(jobs_in_q, lins::curr_timestamp());
                            long double avg_enqueueing_speed{0.0L};
                            if(jobs_enqueuing_timings.size() > 1) {
                                std::vector<long double> speeds{};
                                speeds.reserve(jobs_enqueuing_timings.size());
                                std::pair<std::uint64_t, std::uint64_t> prev_msg{jobs_enqueuing_timings.front()};
                                auto it{jobs_enqueuing_timings.begin()};
                                ++it;
                                for(; it != jobs_enqueuing_timings.end(); ++it) {
                                    std::pair<std::uint64_t, std::uint64_t> const &curr_msg{*it};
                                    long double dn{static_cast<long double>(curr_msg.first) - static_cast<long double>(prev_msg.first)};
                                    if(dn != 0.0L) {
                                        long double dt{(static_cast<long double>(curr_msg.second) - static_cast<long double>(prev_msg.second)) * 1e-6L};
                                        if(dt > 0.0L) {
                                            long double speed{dn / dt};
                                            speeds.push_back(speed);
                                        }
                                    }
                                    prev_msg = curr_msg;
                                }
                                if(!speeds.empty()) {
                                    for(auto &&s: speeds) { avg_enqueueing_speed += s; }
                                    avg_enqueueing_speed /= static_cast<long double>(speeds.size());
                                }
                            }
                            avg_enqueueing_speed_ = avg_enqueueing_speed;


                            {
                                std::unique_lock pl{processors_mtp_};


                                // recalc threads workloads, work time
                                long double avg_workload_percentage{0.0L};
                                if(threads_workloads_.size() != processors_.size()) {
                                    std::unique_lock threads_workloads_lock{threads_workloads_mtp_};
                                    threads_workloads_.resize(processors_.size());
                                }
                                std::size_t curr_threads_workloads_index{0};
                                for(auto &&worker: processors_) {
                                    long double prc{worker.workload_percentage()};
                                    total_work_time_ = total_work_time_ + worker.fetch_work_time();
                                    worker.curr_workload_ = prc;
                                    threads_workloads_[curr_threads_workloads_index++] = prc;
                                    avg_workload_percentage += prc;
                                }
                                if(!processors_.empty()) {
                                    avg_workload_percentage /= static_cast<long double>(processors_.size());
                                }
                                avg_workload_percentage_ = avg_workload_percentage;


                                // spawn when needed
                                bool spawned{false};
                                long double time_since_spawning{lins::curr_timestamp_seconds() - last_thread_spawning_time};
                                long double proc_strength{(long double)processors_.size()};
                                proc_strength = -proc_strength;
                                if(
                                   processors_.size() < max_threads_
                                   &&
                                   jobs_in_q > 0
                                   &&
                                   (
                                       processors_.empty()
                                       ||
                                       (
                                           time_since_spawning > min_seconds_between_spawnings() &&
                                           avg_enqueueing_speed > proc_strength * 20 &&
                                           avg_workload_percentage >= min_workload_to_start_spawn_threads()
                                       )
                                   )
                                ) {
                                    processors_.emplace_back(this);
                                    last_thread_spawning_time = lins::curr_timestamp_seconds();
                                    spawned = true;
                                }


                                // kill when not needed
                                long double time_since_killing{lins::curr_timestamp_seconds() - last_thread_killing_time};
                                if(
                                    (
                                        processors_.size() > max_threads_
                                    )
                                    ||
                                    (
                                        time_since_killing > min_seconds_between_killings() &&
                                        !spawned && jobs_in_q == 0 &&
                                        avg_workload_percentage < max_workload_to_start_kill_threads()
                                    )
                                ) {
                                    for(auto it{processors_.begin()}; it != processors_.end(); ++it) {
                                        if((processors_.size() > max_threads_) ||
                                           (it->curr_workload_ < max_workload_to_start_kill_threads() &&
                                           it->time_of_life() > min_thread_life_time() &&
                                           it->has_worked_since_spawning() &&
                                           it->inactive_seconds() > thread_inactive_seconds_before_kill())
                                        ) {
                                            if(it->prepare_for_stopping()) {
                                                processors_.erase(it);
                                                last_thread_killing_time = lins::curr_timestamp_seconds();
                                                break;
                                            }
                                        }
                                    }
                                }
                            }

                            std::unique_lock al(arbiter_mtp_);
                            if(size_approx_ == 0) {
                                arbiter_cv_.wait_for(al, std::chrono::microseconds{controller_waiting_interval_microseconds()});
                            } else {
                                al.unlock();
                                if(avg_workload_percentage_ > min_workload_to_start_spawn_threads()) {
                                    long double time_to_sleep{min_seconds_between_spawnings() - (lins::curr_timestamp_seconds() - last_thread_spawning_time)};
                                    if(time_to_sleep > 1e-4L) {
                                        lins::thread_base::sleep(time_to_sleep);
                                    }
                                } else if(avg_workload_percentage_ < max_workload_to_start_kill_threads()) {
                                    long double time_to_sleep{min_seconds_between_killings() - (lins::curr_timestamp_seconds() - last_thread_killing_time)};
                                    if(time_to_sleep > 1e-4L) {
                                        lins::thread_base::sleep(time_to_sleep);
                                    }
                                } else {
                                    std::this_thread::sleep_for(std::chrono::microseconds{controller_waiting_interval_microseconds()});
                                }
                            }
                        } catch (...) {
                        }
                    }
                }
            };
        }

        command_queue(command_queue &&) = delete;

        command_queue(command_queue const &) = delete;

        command_queue &operator=(command_queue &&) = delete;

        command_queue &operator=(command_queue const &) = delete;

        ~command_queue() {
            terminate();
            try {
                {
                    std::lock_guard el(evt_mtp_);
                    evt_.notify_all();
                }
                if(arbiter_thread_.joinable()) {
                    arbiter_thread_.join();
                }
                std::unique_lock pl{processors_mtp_};
                processors_.clear();
            } catch(...) {
            }
        }

        std::size_t num_of_threads() const {
            std::shared_lock pl{processors_mtp_};
            return processors_.size();
        }

        std::vector<double> threads_workloads() const {
            std::shared_lock threads_workloads_lock{threads_workloads_mtp_};
            return threads_workloads_;
        }

        std::size_t num_of_jobs_in_queue() const {
            return size_approx_;
        }

        long double threads_total_work_time() const {
            return total_work_time_.load();
        }

        std::uint64_t max_queue_len() const {
            return max_queue_len_;
        }

        void set_max_queue_len(std::size_t len) {
            max_queue_len_ = len;
        }

        std::uint64_t worker_waiting_interval_microseconds() const {
            return worker_waiting_interval_microseconds_;
        }

        void set_worker_waiting_interval_microseconds(std::uint64_t usec) {
            worker_waiting_interval_microseconds_ = usec;
        }

        std::uint64_t controller_waiting_interval_microseconds() const {
            return controller_waiting_interval_microseconds_;
        }

        void set_controller_waiting_interval_microseconds(std::uint64_t usec) {
            controller_waiting_interval_microseconds_ = usec;
        }

        long double min_seconds_between_spawnings() const {
            return min_seconds_between_spawnings_;
        }

        void set_min_seconds_between_spawnings(long double tps) {
            min_seconds_between_spawnings_ = tps;
        }

        long double min_seconds_between_killings() const {
            return min_seconds_between_killings_;
        }

        void set_min_seconds_between_killings(long double tps) {
            min_seconds_between_killings_ = tps;
        }

        std::uint64_t thread_inactive_seconds_before_kill() const {
            return thread_inactive_seconds_before_kill_;
        }

        void set_thread_inactive_seconds_before_kill(std::uint64_t sec) {
            thread_inactive_seconds_before_kill_ = sec;
        }

        long double max_workload_to_start_kill_threads() const {
            return max_workload_to_start_kill_threads_;
        }

        void set_max_workload_to_start_kill_threads(long double wl) {
            max_workload_to_start_kill_threads_ = wl;
        }

        long double min_workload_to_start_spawn_threads() const {
            return min_workload_to_start_spawn_threads_;
        }

        void set_min_workload_to_start_spawn_threads(double wl) {
            min_workload_to_start_spawn_threads_ = wl;
        }

        long double min_thread_life_time() const {
            return min_thread_life_time_;
        }

        void set_min_thread_life_time(long double v) {
            min_thread_life_time_ = v;
        }

        long double queueing_dynamics() const {
            return avg_enqueueing_speed_;
        }

        long double threads_workload() const {
            return avg_workload_percentage_;
        }

        std::size_t threads() const {
            return processors_.size();
        }

        bool add_job(std::function<void()> &&j) {
            while(size_approx_ >= max_queue_len_) {
                std::this_thread::sleep_for(std::chrono::milliseconds{1});
            }
            bool result{true};
            {
                std::unique_lock el{evt_mtp_};
                jobs_.push_back(std::move(j));
                size_approx_update();
                evt_.notify_one();
            }
            if(result) {
                std::unique_lock al(arbiter_mtp_);
                arbiter_cv_.notify_one();
            }
            return result;
        }

        bool add_job(std::function<void()> const &j) {
            while(size_approx_ >= max_queue_len_) {
                std::this_thread::sleep_for(std::chrono::milliseconds{1});
            }
            bool result{true};
            {
                std::unique_lock el(evt_mtp_);
                jobs_.push_back(j);
                size_approx_update();
                evt_.notify_one();
            }
            if(result) {
                std::unique_lock al(arbiter_mtp_);
                arbiter_cv_.notify_one();
            }
            return result;
        }

        bool add_job_urgent(std::function<void()> &&j) {
            while(size_approx_ >= max_queue_len_) {
                std::this_thread::sleep_for(std::chrono::milliseconds{1});
            }
            bool result{true};
            {
                std::unique_lock el{evt_mtp_};
                hp_jobs_.push_back(std::move(j));
                size_approx_update();
                evt_.notify_one();
            }
            if(result) {
                std::unique_lock al(arbiter_mtp_);
                arbiter_cv_.notify_one();
            }
            return result;
        }

        bool add_job_urgent(std::function<void()> const &j) {
            while(size_approx_ >= max_queue_len_) {
                std::this_thread::sleep_for(std::chrono::milliseconds{1});
            }
            bool result{true};
            {
                std::unique_lock el{evt_mtp_};
                hp_jobs_.push_back(j);
                size_approx_update();
                evt_.notify_one();
            }
            if(result) {
                std::unique_lock al(arbiter_mtp_);
                arbiter_cv_.notify_one();
            }
            return result;
        }

    private:
        bool fetch_job(
            std::function<void()> &j,
            std::function<void(std::int64_t)> const &log_wait_time,
            std::function<bool()> const &lock_for_processing,
            std::function<bool()> const &unlock_for_stop
        ) {
            std::unique_lock el(evt_mtp_);
            while(!termination()) {
                if(lock_for_processing()) {
                    {
                        lins::shut_on_destroy unlocker{[&]() { unlock_for_stop(); }};
                        if(hp_jobs_.size() > 0) {
                            j = std::move(hp_jobs_.front());
                            hp_jobs_.pop_front();
                            size_approx_update();
                            return true;
                        }
                        if(jobs_.size() > 0) {
                            j = std::move(jobs_.front());
                            jobs_.pop_front();
                            size_approx_update();
                            return true;
                        }
                    }
                    std::uint64_t beg_wait_ts{lins::curr_timestamp()};
                    if(!termination() && jobs_size_approx() == 0) {
                        evt_.wait_for(el, std::chrono::microseconds{worker_waiting_interval_microseconds()});
                    }
                    std::uint64_t end_wait_ts{lins::curr_timestamp()};
                    log_wait_time(static_cast<std::int64_t>(beg_wait_ts) - static_cast<std::int64_t>(end_wait_ts));
                } else {
                    return false;
                }
            }
            return false;
        }

        void size_approx_update() {
            size_approx_.store(jobs_.size() + hp_jobs_.size());
        }

        std::size_t jobs_size_approx() const {
            return size_approx_;
        }

        class worker: public terminable {
            friend class command_queue;

            void log_wait_time_arg(std::int64_t dt) { log_wait_time(dt); has_worked_since_spawning_ = true; }
            bool lock_for_processing_arg() { return lock_for_processing(); }
            bool unlock_for_stop_arg() { return unlock_for_stop(); }

            std::atomic<std::uint64_t> bgn_wrk_{};

            void update_wrk_time(bool force = false) {
                if(busy_ || force) {
                    std::uint64_t bgn_wrk{bgn_wrk_.exchange(lins::curr_timestamp())};
                    std::int64_t wrk_time{static_cast<std::int64_t>(lins::curr_timestamp()) - static_cast<std::int64_t>(bgn_wrk)};
                    overall_work_time_ = overall_work_time_ + wrk_time / 1000000.0L;
                    {
                        std::unique_lock tl{times_mtp_};
                        time_intervals_.push_back(wrk_time);
                    }
                    last_activity_time_ = lins::curr_timestamp_seconds();
                }
            }

        public:
            worker(command_queue *owner):
                owner_{owner},
                work_thread_{
                    [this]() {
                        while(!termination() && !owner_->termination()) {
                            if(!doomed()) {
                                std::function<void()> j;
                                if(
                                    owner_->fetch_job(j,
                                        std::bind(&worker::log_wait_time_arg, this, std::placeholders::_1),
                                        std::bind(&worker::lock_for_processing_arg, this),
                                        std::bind(&worker::unlock_for_stop_arg, this)
                                    )
                                ) {
                                    bgn_wrk_ = lins::curr_timestamp();
                                    busy_.store(true);
                                    last_activity_time_ = lins::curr_timestamp_seconds();
                                    try { j(); } catch(...) {}
                                    busy_.store(false);
                                    update_wrk_time(true);
                                    has_worked_since_spawning_ = true;
                                }
                            } else {
                                lins::thread_base::sleep(0.01);
                            }
                        }
                    }
                }
            {
            }

            ~worker() {
                terminate();
                try {
                    if(work_thread_.joinable()) {
                        work_thread_.join();
                    }
                } catch(...) {
                }
            }

            long double fetch_work_time() {
                return overall_work_time_.exchange(0.0L);
            }

            double workload_percentage() {
                long double res{0.0L};
                update_wrk_time();
                std::shared_lock tl{times_mtp_};
                if(time_intervals_.size() > 0) {
                    auto time_intervals{time_intervals_};
                    tl.unlock();
                    long double sum{0.0L};
                    long double wrk{0.0L};
                    for(auto &&dt: time_intervals) {
                        sum += std::abs(static_cast<long double>(dt));
                        if(dt > 0) {
                            wrk += static_cast<long double>(dt);
                        }
                    }
                    if(sum > 0) {
                        res = wrk / sum;
                    }
                }
                return res;
            }

            std::thread::id id() const noexcept {
                return work_thread_.get_id();
            }

            long double time_of_life() const noexcept {
                return lins::curr_timestamp_seconds() - spawned_time_;
            }

            bool lock_for_processing() noexcept {
                int cpe{0};
                return work_or_stop_pending_.compare_exchange_strong(cpe, -1);
            }

            bool unlock_for_stop() noexcept {
                int cpe{-1};
                return work_or_stop_pending_.compare_exchange_strong(cpe, 0);
            }

            bool doomed() const noexcept {
                return work_or_stop_pending_.load() == 1;
            }

            bool prepare_for_stopping() noexcept {
                int cpe{0};
                return work_or_stop_pending_.compare_exchange_strong(cpe, 1);
            }

            long double inactive_seconds() const {
                if(busy_.load()) {
                    last_activity_time_ = lins::curr_timestamp_seconds();
                }
                return lins::curr_timestamp_seconds() - last_activity_time_;
            }

            bool busy() const {
                if(busy_.load()) {
                    last_activity_time_ = lins::curr_timestamp_seconds();
                    return true;
                }
                return false;
            }

            bool has_worked_since_spawning() const {
                return has_worked_since_spawning_.load();
            }

        private:
            void log_wait_time(std::int64_t dt) {
                std::unique_lock tl{times_mtp_};
                time_intervals_.push_back(dt);
            }

        private:
            alignas(64) command_queue *owner_{nullptr};
            alignas(64) std::thread work_thread_;
            alignas(64) mutable long double spawned_time_{curr_timestamp_seconds()};
            alignas(64) mutable long double last_activity_time_{0};
            alignas(64) std::atomic_bool busy_{false};
            alignas(64) std::atomic<int> work_or_stop_pending_{0};
            alignas(64) lins::ring_buffer<std::int64_t, 16> time_intervals_{};
            alignas(64) mutable lins::dbg_shared_mutex times_mtp_{};
            alignas(64) std::atomic<long double> overall_work_time_{0.0L};
            alignas(64) std::atomic<long double> curr_workload_{0.0L};
            alignas(64) std::atomic_bool has_worked_since_spawning_{false};
        };

    private:
        long double max_workload_to_start_kill_threads_{0.01};
        long double min_workload_to_start_spawn_threads_{0.80};
        long double min_seconds_between_spawnings_{0};
        long double min_seconds_between_killings_{1};
        long double min_thread_life_time_{10.0L};
        std::uint64_t thread_inactive_seconds_before_kill_{30};
        std::uint64_t worker_waiting_interval_microseconds_{50'000};
        std::uint64_t controller_waiting_interval_microseconds_{50'000};

        std::size_t max_threads_{std::thread::hardware_concurrency() * 2};
        std::thread arbiter_thread_{};
        std::condition_variable arbiter_cv_{};
        mutable std::mutex arbiter_mtp_{};

        std::size_t max_queue_len_{10'000'000};
        std::list<std::function<void()>> jobs_{};
        std::list<std::function<void()>> hp_jobs_{};
        std::atomic<std::size_t> size_approx_{0};
        mutable std::mutex evt_mtp_{};
        std::condition_variable evt_{};
        mutable lins::dbg_shared_mutex processors_mtp_{};
        std::list<worker> processors_{};

        std::atomic<long double> avg_enqueueing_speed_{0.0L};
        long double avg_workload_percentage_{0.0L};

        std::atomic<long double> total_work_time_{0.0L};

        std::vector<double> threads_workloads_{};
        mutable lins::dbg_shared_mutex threads_workloads_mtp_{};
    };


    class deferred_command_queue final: public terminable {
    public:
        deferred_command_queue(std::size_t thrd_cnt, std::size_t max_len = 10'000);
        deferred_command_queue(deferred_command_queue &&) = delete;
        deferred_command_queue(deferred_command_queue const &) = delete;
        deferred_command_queue &operator=(deferred_command_queue &&) = delete;
        deferred_command_queue &operator=(deferred_command_queue const &) = delete;
        ~deferred_command_queue();
        void add_job_timeout(std::function<void()> &&, long double timeout_seconds);
        void add_job_timeout(std::function<void()> const &, long double timeout_seconds);
        void add_job_gmt_fire_time(std::function<void()> &&, lins::timespec_wrapper const &fire_time);
        void add_job_gmt_fire_time(std::function<void()> const &, lins::timespec_wrapper const &fire_time);

    private:
        bool fetch_job(std::function<void()> &);

        std::size_t jobs_size_approx() const {
            return approx_size_;
        }

    private:
        std::size_t max_len_{10'000};
        std::map<long double, std::function<void()>> jobs_;
        std::atomic<std::size_t> approx_size_{0};
        std::condition_variable evt_{};
        mutable std::mutex evt_mtp_{};
        lins::dbg_mutex processors_mtp_{};
        std::vector<std::thread> processors_{};
    };

}
