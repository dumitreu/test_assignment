#pragma once

#include "../commondefs.hpp"
#include "../timespec_wrapper.hpp"
#include "../bit_util.hpp"
#include "errno_except.hpp"
#ifndef PLATFORM_WINDOWS
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

namespace lins::net {

    DEFINE_RUNTIME_EXCEPTION_CLASS(socket_error)

    class socket final {
    public:
        socket() = default;
        socket(socket const &) = delete;
        socket &operator=(socket const &) = delete;
        socket(socket &&) = delete;
        socket &operator=(socket &&) = delete;
        ~socket();
        bool create();
        bool bind(const std::string &address, std::uint16_t port);
        bool listen(int que = SOMAXCONN);
        void accept(socket &client_sock);
        bool connect(const std::string &address, std::uint16_t port);
        std::vector<std::uint8_t> receive(int len);
        int send(void const *buff, int len, int flags
#ifdef PLATFORM_WINDOWS
            = 0
#else
            = MSG_NOSIGNAL
#endif
        );
        bool close() noexcept;
        bool shutdown();
        int  handle() const noexcept;
        operator int() const noexcept;
        bool make_nonblocking();
        bool set_reuse_addr();
        bool make_nodelay();
        bool make_nosigpipe();
        bool set_linger(bool on, int linger_time);
        bool set_keepalive(bool keep);
        bool get_keepalive() const;
        bool set_tcp_keepidle(int);
        int get_tcp_keepidle() const;
        bool set_tcp_keepitvl(int);
        int get_tcp_keepitvl() const;
        bool set_tcp_keepcnt(int);
        int get_tcp_keepcnt() const;
        bool set_rcv_timeout(lins::timespec_wrapper const &to, lins::timespec_wrapper &orig_to);
        std::string peer_addr() const noexcept;
        bool ok() const noexcept {
            if(sock_fd_ >= 0) {
                int error_code{};
#ifdef PLATFORM_WINDOWS
                int error_code_size{sizeof(error_code)};
                int qres{::getsockopt(sock_fd_, SOL_SOCKET, SO_ERROR, (char *) & error_code, &error_code_size)};
#else
                socklen_t error_code_size{sizeof(error_code)};
                int qres{::getsockopt(sock_fd_, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size)};
#endif
                return qres == 0 && error_code == 0;
            }
            return false;
        }

        int send_message(std::vector<std::uint8_t> const &data) {
            return send_message(data.data(), data.size());
        }

        int send_message(const void *data, std::uint32_t data_size) {
            int res{};
            if(data && data_size > 0) {
                std::uint32_t net_size = lins::bit_util::hnswap<std::uint32_t>{data_size}.val;
                std::uint32_t total_size = sizeof(std::uint32_t);
                std::uint32_t total_sent = 0;
                const char *curr_buff = reinterpret_cast<const char *>(&net_size);
                do {
                    int wrote_count = this->send(curr_buff + total_sent, total_size - total_sent);
                    if(wrote_count < 0) {
                        int e{errno};
                        while(e == EAGAIN) {
                            wrote_count = this->send(curr_buff + total_sent, total_size - total_sent);
                            if(wrote_count < 0) {
                                e = errno;
                            } else {
                                e = 0;
                            }
                        }
                        if(e != 0) {
                            throw_errno(errno);
                        }
                    }
                    total_sent += wrote_count;
                } while(total_sent != total_size);
                total_size = data_size;
                total_sent = 0;
                curr_buff = reinterpret_cast<const char *>(data);
                do {
                    int wrote_count = this->send(curr_buff + total_sent, total_size - total_sent);
                    if(wrote_count < 0) {
                        int e{errno};
                        while(e == EAGAIN) {
                            wrote_count = this->send(curr_buff + total_sent, total_size - total_sent);
                            if(wrote_count < 0) {
                                e = errno;
                            } else {
                                e = 0;
                            }
                        }
                        if(e != 0) {
                            throw_errno(errno);
                        }
                    }
                    total_sent += wrote_count;
                } while(total_sent != total_size);
                res = total_sent;
            }
            return res;
        }

#if 1
    std::vector<std::uint8_t> receive_message() {
        std::vector<std::uint8_t> result{};
        if(ok()) {
            std::vector<std::uint8_t> msg_size_vec{};
            for(msg_size_vec = lins::net::socket::receive(4); msg_size_vec.size() < 4; ) {
                std::vector<std::uint8_t> msg_size_vec_part{lins::net::socket::receive(4 - msg_size_vec.size())};
                if(msg_size_vec_part.size() > 0) {
                    msg_size_vec.insert(msg_size_vec.end(), msg_size_vec_part.begin(), msg_size_vec_part.end());
                }
            }
            std::uint32_t msg_size{};
            if(msg_size_vec.size() == sizeof(msg_size)) {
                std::memcpy(&msg_size, msg_size_vec.data(), 4);
                std::uint32_t msg_size_host{lins::bit_util::hnswap{msg_size}.val};
                for(result = lins::net::socket::receive(msg_size_host); result.size() < msg_size_host; ) {
                    std::vector<std::uint8_t> data_partial_vec{lins::net::socket::receive(msg_size_host - result.size())};
                    if(data_partial_vec.size() > 0) {
                        result.insert(result.end(), data_partial_vec.begin(), data_partial_vec.end());
                    }
                }
            }
        } else {
            throw std::runtime_error{"receive_message(): invalid socket state"};
        }
        return result;
    }
#endif

    protected:
    #ifndef _WIN32
        bool set_fd_flag(unsigned int flag);
        bool reset_fd_flag(unsigned int flag);
    #endif

    protected:
        int sock_fd_{-1};
        struct sockaddr_in sock_addr_{};
    };

}
