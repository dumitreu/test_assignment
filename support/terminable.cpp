#include "terminable.hpp"

namespace lins {

    void terminable::terminate() noexcept {
        termination_requested_.store(true, std::memory_order_seq_cst);
    }

    void terminable::unterminate() noexcept {
        termination_requested_.store(false, std::memory_order_seq_cst);
    }

    bool terminable::termination() const noexcept {
        return termination_requested_.load(std::memory_order_acquire);
    }

}
