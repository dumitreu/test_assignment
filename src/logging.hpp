#pragma once

#include <commondefs.hpp>
#include <threading.hpp>
#include <timespec_wrapper.hpp>
#include <file_util.hpp>

namespace logging {

    extern std::unique_ptr<lins::thread_safe_wrapper_stl<std::ostream *>> log_output;

    void init();
    void set_log(std::ostream *);
    void set_log_null();

}
#ifdef PLATFORM_WINDOWS
#define LOG (****logging::log_output) << lins::timespec_wrapper::now().to_iso_8601_str(3) << " in file " << lins::file_util::extract_file_name(__FILE__) << ":" << __LINE__ << ", " << __FUNCSIG__ << "(): "
#else
#define LOG (****logging::log_output) << lins::timespec_wrapper::now().to_iso_8601_str(3) << " in file " << lins::file_util::extract_file_name(__FILE__) << ":" << __LINE__ << ", " << __PRETTY_FUNCTION__ << "(): "
#endif
