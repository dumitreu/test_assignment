#pragma once

#include <commondefs.hpp>
#include <threading.hpp>
#include <command_queue.hpp>

#include "configuration.hpp"

extern bool global_termination;

extern std::unique_ptr<lins::command_queue> global_regular_queue;
extern std::unique_ptr<lins::command_queue> global_retry_queue;

extern std::unique_ptr<lins::thread_safe_wrapper_stl<configuration>> global_config;

lins::thread_safe_wrapper_stl<configuration> &config();
