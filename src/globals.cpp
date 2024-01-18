#include "globals.hpp"

bool global_termination{false};

std::unique_ptr<lins::command_queue> global_regular_queue{};
std::unique_ptr<lins::command_queue> global_retry_queue{};

std::unique_ptr<lins::thread_safe_wrapper_stl<configuration>> global_config{};

lins::thread_safe_wrapper_stl<configuration> &config() {
    return *global_config;
}
