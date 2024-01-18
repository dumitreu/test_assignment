#pragma once

#include "../commondefs.hpp"
#include "../timespec_wrapper.hpp"

namespace lins {

    template<typename T>
    class scoped_locker {
        friend class condition_variable;

    public:
        scoped_locker() = default;

        scoped_locker(const scoped_locker &that) = delete;

        scoped_locker &operator=(const scoped_locker &that) = delete;

        scoped_locker(scoped_locker &&rv) = delete;

        scoped_locker &operator=(scoped_locker &&rv) = delete;

        ~scoped_locker() {
            unlock();
        }

        scoped_locker(T const *l, bool kind = true): l_{const_cast<T *>(l)}, kind_{kind} {
            if(!lock(eternity, kind)) {
                throw std::runtime_error("scoped_locker failed to lock");
            }
        }
        scoped_locker(T &l, bool kind = true): l_{&l}, kind_(kind) {
            if(!lock(eternity, kind)) {
                throw std::runtime_error("scoped_locker failed to lock");
            }
        }

        void adopt(T *l, bool kind = true) noexcept {
            unlock();
            l_ = l;
            kind_ = kind;
        }

        void adopt(T &l, bool kind = true) noexcept {
            unlock();
            l_ = &l;
            kind_ = kind;
        }

        bool lock(timespec_wrapper const &timeout = eternity, bool w = true) {
            return l_->lock(timeout, w);
        }

        bool unlock() noexcept {
            if(!l_) {
                return true;
            }
            T *l{l_};
            l_ = nullptr;
            return l->unlock(kind_);
        }

        bool upgrade() noexcept {
            if(kind_) {
                return true;
            }
            kind_ = true;
            return l_->upgrade();
        }

        bool downgrade() noexcept {
            if(!kind_) {
                return true;
            }
            kind_ = false;
            return l_->downgrade();
        }

    private:
        T *l_{nullptr};
        bool kind_{true};
    };

}
