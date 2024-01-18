#pragma once

#include "commondefs.hpp"

namespace lins {

    class terminable {
    public:
        terminable() = default;
        ~terminable() = default;
        void terminate() noexcept;
        void unterminate() noexcept;
        bool termination() const noexcept;

    protected:
        std::atomic_bool termination_requested_{false};
    };

}
