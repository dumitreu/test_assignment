#include "../commondefs.hpp"
#include "socket_wrapper.hpp"
#include "errno_except.hpp"
#include "../str_util.hpp"
#include "../bit_util.hpp"
#include <fcntl.h>
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID) || defined(PLATFORM_POSIX) || defined(PLATFORM_UNIXISH) || defined(PLATFORM_APPLE)
#include <netinet/tcp.h>
#endif

namespace lins::net {

    socket::~socket() {
        try {
            if(ok()) {
                shutdown();
            }
        } catch(...) {
        }
        close();
    }

    bool socket::create() {
        if(ok()) {
            throw socket_error("create() call on already initialized socket");
        }
        if((sock_fd_ = ::socket(AF_INET, SOCK_STREAM, 0)) > 0) {
            return true;
        }
        return false;
    }

    bool socket::bind(const std::string &address, std::uint16_t port) {
        if(ok()) {
            sock_addr_.sin_family = AF_INET;
            sock_addr_.sin_port = lins::bit_util::hnswap<std::uint16_t>{static_cast<std::uint16_t>(port)}.val;
            sock_addr_.sin_addr.s_addr = inet_addr(address.c_str());

            if(::bind(sock_fd_, (struct sockaddr *)&sock_addr_, sizeof(struct sockaddr)) == 0) {
                return true;
            }
        }
        return false;
    }

    bool socket::listen(int que) {
        if(ok()) {
            if(::listen(sock_fd_, que) == 0) {
                return true;
            }
        } else {
            throw socket_error("listen() call on uninitialized socket");
        }
        return false;
    }

#ifdef PLATFORM_WINDOWS
    typedef int socklen_t;
#endif

    void socket::accept(socket &client_sock) {
        if(ok()) {
            socklen_t size = sizeof(struct sockaddr);
            client_sock.sock_fd_ = ::accept(sock_fd_, (struct sockaddr *)&client_sock.sock_addr_, &size);
            if(client_sock.sock_fd_ == -1) {
                throw_errno(errno);
            }
        } else {
            throw socket_error("socket::accept() call on uninitialized socket");
        }
    }

    bool socket::connect(const std::string &address, std::uint16_t port) {
        if(ok()) {
            struct in_addr *addr_ptr;
            struct hostent *hostPtr;
            std::string add;
            hostPtr = ::gethostbyname(address.c_str());
            if(hostPtr == NULL) {
                return false;
            }
            addr_ptr = (struct in_addr *)*hostPtr->h_addr_list;
            if(addr_ptr) {
                add = ::inet_ntoa(*addr_ptr);
                if(add == "") {
                    return false;
                }
                struct sockaddr_in sock_addr;
                sock_addr.sin_family = AF_INET;
                sock_addr.sin_port = lins::bit_util::hnswap<std::uint16_t>{static_cast<std::uint16_t>(port)}.val;
                sock_addr.sin_addr.s_addr = inet_addr(add.c_str());
                auto conn_res{::connect(sock_fd_, (struct sockaddr *)&sock_addr, sizeof(struct sockaddr))};
                if(conn_res == 0) {
                    return true;
                }
            }
        } else {
            throw socket_error("connect() call on uninitialized socket");
        }
        return false;
    }

    std::vector<std::uint8_t> socket::receive(int len) {
        std::vector<std::uint8_t> result{};
        if(ok()) {
            if(len <= 0) {
                throw socket_error("wrong arguments in call to socket::receive()");
            }
            result.resize(len);
            int rd = recv(sock_fd_, reinterpret_cast<char *>(&result[0]), len, 0);
            if(rd >= 0) {
                if(rd != len) { result.resize(rd); }
            } else {
                throw_errno(errno);
            }
        } else {
            throw socket_error("socket::receive(): socket not ready");
        }
        return result;
    }

    int socket::send(const void *buff, int len) {
        if(ok()) {
            if(len <= 0 || !buff) {
                throw socket_error("wrong arguments in call to socket::send()");
            }
            return ::send(sock_fd_, reinterpret_cast<const char *>(buff), len, 0);
        } else {
            throw socket_error("socket::send(): socket not ready");
        }
    }

    bool socket::close() noexcept {
        int close_res{-1};
        if(ok()) {
#ifdef PLATFORM_WINDOWS
            close_res = ::closesocket(sock_fd_);
#else
            close_res = ::close(sock_fd_);
#endif
            if(close_res == 0) { sock_fd_ = -1; }
        }
        return close_res == 0;
    }

    bool socket::shutdown() {
        if(ok()) {
            return ::shutdown(sock_fd_,
#ifdef PLATFORM_WINDOWS
                /*SD_BOTH*/0x02
#else
                SHUT_RDWR
#endif
                ) == 0;
        } else {
            throw socket_error("socket::shutdown(): socket not ready");
        }
        return false;
    }

    int socket::handle() const noexcept {
        return sock_fd_;
    }

    socket::operator int() const noexcept {
        return handle();
    }

    bool socket::make_nonblocking() {
        if(!ok()) {
            throw socket_error("socket::make_nonblocking(): socket not ready");
        }
#ifdef PLATFORM_WINDOWS
        // Set the socket I/O mode: In this case FIONBIO
        // enables or disables the blocking mode for the
        // socket based on the numerical value of iMode.
        // If iMode = 0, blocking is enabled;
        // If iMode != 0, non-blocking mode is enabled.
        u_long iMode = 1;
        return ioctlsocket(sock_fd_, FIONBIO, &iMode) == NO_ERROR;
#else
        return set_fd_flag(O_NONBLOCK);
#endif
    }

    bool socket::set_reuse_addr() {
        if(!ok()) {
            throw socket_error("socket::set_reuse_addr(): socket not ready");
        }
#ifdef PLATFORM_WINDOWS
        int val{ 1 };
        return ::setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEADDR, (char const *)&val, sizeof(int)) >= 0;
#else
        int val{1};
        return ::setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int)) >= 0;
#endif
    }

    bool socket::make_nodelay() {
        if(!ok()) {
            throw socket_error("socket::make_nodelay(): socket not ready");
        }
#ifdef PLATFORM_WINDOWS
        BOOL b = TRUE;
        return ::setsockopt(sock_fd_, IPPROTO_TCP, TCP_NODELAY, (char const *)&b, sizeof(BOOL)) == 0;
#else
        return set_fd_flag(TCP_NODELAY);
#endif
    }

    bool socket::set_linger(bool on, int linger_time) {
        if(!ok()) {
            throw socket_error("socket::make_nodelay(): socket not ready");
        }
        struct linger sl{};
        sl.l_onoff = on ? 1 : 0;
        sl.l_linger = linger_time;
        return ::setsockopt(sock_fd_, SOL_SOCKET, SO_LINGER, (char const *)&sl, sizeof(sl)) == 0;
    }

    bool socket::set_keepalive(bool keep) {
        if(!ok()) {
            throw socket_error("socket::set_keepalive(): socket not ready");
        }
        int flags{keep ? 1 : 0};
        return ::setsockopt(sock_fd_, SOL_SOCKET, SO_KEEPALIVE, (char const *)&flags, sizeof(flags)) == 0;
    }

    bool socket::get_keepalive() const {
        if(!ok()) {
            throw socket_error("socket::get_keepalive(): socket not ready");
        }
        socklen_t len = sizeof(int);
        int flags{};
        if(::getsockopt(sock_fd_, SOL_SOCKET, SO_KEEPALIVE, (char *)&flags, &len) != 0) {
            throw socket_error("socket::get_keepalive(): failed to query option");
        }
        return flags;
    }

    // overrides net.ipv4.tcp_keepalive_time
    bool socket::set_tcp_keepidle(int val) {
        if(!ok()) {
            throw socket_error("socket::set_tcp_keepidle(): socket not ready");
        }
#ifndef PLATFORM_APPLE
        int idle = val;
        return ::setsockopt(sock_fd_, IPPROTO_TCP, TCP_KEEPIDLE, (char const *)&idle, sizeof(int)) == 0;
#else
        return true;
#endif
    }

    int socket::get_tcp_keepidle() const {
        if(!ok()) {
            throw socket_error("socket::get_tcp_keepidle(): socket not ready");
        }
#ifndef PLATFORM_APPLE
        socklen_t len = sizeof(int);
        int val{};
        if(::getsockopt(sock_fd_, IPPROTO_TCP, TCP_KEEPIDLE, (char *)&val, &len) != 0) {
            throw socket_error("socket::get_tcp_keepidle(): failed to query option");
        }
        return val;
#else
        return -1;
#endif
    }

    // overrides net.ipv4. tcp_keepalive_intvl
    bool socket::set_tcp_keepitvl(int val) {
        if(!ok()) {
            throw socket_error("socket::set_tcp_keepitvl(): socket not ready");
        }
        int interval = val;
        return ::setsockopt(sock_fd_, IPPROTO_TCP, TCP_KEEPINTVL, (char const *)&interval, sizeof(int)) == 0;
    }

    int socket::get_tcp_keepitvl() const {
        if(!ok()) {
            throw socket_error("socket::get_tcp_keepitvl(): socket not ready");
        }
        socklen_t len = sizeof(int);
        int val{};
        if(::getsockopt(sock_fd_, IPPROTO_TCP, TCP_KEEPINTVL, (char *)&val, &len) != 0) {
            throw socket_error("socket::get_tcp_keepitvl(): failed to query option");
        }
        return val;
    }

    // overrides net.ipv4.tcp_keepalive_probes
    bool socket::set_tcp_keepcnt(int val) {
        if(!ok()) {
            throw socket_error("socket::set_tcp_keepcnt(): socket not ready");
        }
        int maxpkt = val;
        return ::setsockopt(sock_fd_, IPPROTO_TCP, TCP_KEEPCNT, (char const *)&maxpkt, sizeof(int)) == 0;
    }

    int socket::get_tcp_keepcnt() const {
        if(!ok()) {
            throw socket_error("socket::get_tcp_keepcnt(): socket not ready");
        }
        socklen_t len = sizeof(int);
        int val{};
        if(::getsockopt(sock_fd_, IPPROTO_TCP, TCP_KEEPCNT, (char *)&val, &len) != 0) {
            throw socket_error("socket::get_tcp_keepcnt(): failed to query option");
        }
        return val;
    }

    // on windows this should be done before calling bind !!!
    bool socket::set_rcv_timeout(lins::timespec_wrapper const &to, lins::timespec_wrapper &orig_to) {
        bool result{false};
        if(sock_fd_ != -1) {
#if defined(PLATFORM_LINUX) || defined(PLATFOM_APPLE)
        struct timeval org_tv{to.seconds(), to.useconds()};
        socklen_t org_tv_len{0};
        if(::getsockopt(sock_fd_, SOL_SOCKET, SO_RCVTIMEO, &org_tv, &org_tv_len) == 0) {
            orig_to = lins::timespec_wrapper{(long double)org_tv.tv_sec + (long double)org_tv.tv_usec / 1'000'000.0L};
            struct timeval tv{to.seconds(), to.useconds()};
            if(::setsockopt(sock_fd_, SOL_SOCKET, SO_RCVTIMEO, (char const *)&tv, sizeof(tv))) {
                result = true;
            }
        }
#elif defined(PLATFOM_WINDOWS)
            DWORD timeout = to.seconds() * 1000;
            ::setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
#endif
        }
        return result;
    }

#ifndef PLATFORM_WINDOWS
    bool socket::set_fd_flag(unsigned int flag) {
        if(!ok()) {
            throw socket_error("socket::set_fd_flag(): socket not ready");
        }
        int flags, s;
        flags = ::fcntl(sock_fd_, F_GETFL, 0);
        if (flags == -1) {
            return false;
        }
        flags |= flag;
        s = ::fcntl (sock_fd_, F_SETFL, flags);
        if (s == -1) {
            return false;
        }
        return true;
    }

    bool socket::reset_fd_flag(unsigned int flag) {
        if(!ok()) {
            throw socket_error("socket::reset_fd_flag(): socket not ready");
        }
        int flags, s;
        flags = fcntl(sock_fd_, F_GETFL, 0);
        if (flags == -1) {
            return false;
        }
        flags &= ~flag;
        s = ::fcntl(sock_fd_, F_SETFL, flags);
        if (s == -1) {
            return false;
        }
        return true;
    }
#endif

    std::string socket::peer_addr() const noexcept {
        return lins::str_util::utoa(((sock_addr_.sin_addr.s_addr) & 0xff))
                + "." + lins::str_util::utoa(((sock_addr_.sin_addr.s_addr >> 8) & 0xff))
                + "." + lins::str_util::utoa(((sock_addr_.sin_addr.s_addr >> 16) & 0xff))
                + "." + lins::str_util::utoa(((sock_addr_.sin_addr.s_addr >> 24) & 0xff))
        ;
    }

}
