#include "logging.hpp"
#include "globals.hpp"

namespace logging {

    template <class charT, class Traits=std::char_traits<charT> >
    class null_ostream: public std::basic_ostream <charT, Traits> {
    public:
        using std::ostream::ostream;
    };

    std::unique_ptr<lins::thread_safe_wrapper_stl<std::ostream *>> log_output{};

    void init() {
        log_output = std::make_unique<lins::thread_safe_wrapper_stl<std::ostream *>>(&std::cout);
    }

    void set_log(std::ostream *os) {
        (***log_output) = os;
    }

    static std::unique_ptr<null_ostream<char>> nullout{};

    void set_log_null() {
        if(!nullout) {
            nullout = std::make_unique<null_ostream<char>>(nullptr);
        }
        (***log_output) = (std::ostream *)nullout.get();
    }

}
