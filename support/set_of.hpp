#pragma once

#include <set>
#include <vector>

namespace lins {

    template<typename T>
    class set_of {
    public:
        set_of() =default;

        set_of(T const &v) {
            v_.insert(v);
        }

        set_of(T &&v) {
            v_.insert(std::move(v));
        }

        set_of(std::vector<T> const &v) {
            for(auto &&i: v) {
                v_.insert(i);
            }
        }

        template<typename... UU>
        set_of(T const &v, UU&&...vv) {
            v_.insert(v);
            set_of oth{std::forward<UU>(vv)...};
            v_.insert(oth.v_.begin(), oth.v_.end());
        }

        template<typename... UU>
        set_of(T &&v, UU&&...vv) {
            v_.insert(std::move(v));
            set_of oth{std::forward<UU>(vv)...};
            v_.insert(oth.v_.begin(), oth.v_.end());
        }

        set_of &operator<<(T const &v) {
            v_.insert(v);
            return *this;
        }

        set_of &operator<<(T &&v) {
            v_.insert(std::move(v));
            return *this;
        }

        set_of &operator-=(set_of const &s) {
            std::set<T> v{};
            for(auto &&item: v_) {
                if(s.v_.find(item) == s.v_.end()) {
                    v.insert(item);
                }
            }
            v_ = std::move(v);
            return *this;
        }

        friend set_of operator-(set_of const &l, set_of const &r) {
            set_of res{l};
            res -= r;
            return res;
        }

        set_of &operator+=(set_of const &s) {
            std::set<T> v{v_};
            for(auto &&item: s.v_) {
                if(v.find(item) == v.end()) {
                    v.insert(item);
                }
            }
            v_ = std::move(v);
            return *this;
        }

        friend set_of operator+(set_of const &l, set_of const &r) {
            set_of res{l};
            res += r;
            return res;
        }

        set_of &operator&=(set_of const &s) {
            std::set<T> v{};
            for(auto &&item: v_) {
                if(s.v_.find(item) != s.v_.end()) {
                    v.insert(item);
                }
            }
            v_ = std::move(v);
            return *this;
        }

        friend set_of operator&(set_of const &l, set_of const &r) {
            set_of res{l};
            res &= r;
            return res;
        }

        bool contains(T const &v) const {
            return v_.find(v) != v_.end();
        }

        void clear() {
            v_.clear();
        }

        std::vector<T> elements() const {
            return std::vector<T>{v_.begin(), v_.end()};
        }

    private:
        std::set<T> v_;
    };

}
