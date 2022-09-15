//
// Created by MrMam on 14.09.2022.
//

#ifndef SEQ_GENERATOR_CLIENT_HXX
#define SEQ_GENERATOR_CLIENT_HXX

#include "general.hxx"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>

class client final : public client_base {
    static size_t constexpr _buffer_size = 1024;
    sockaddr_in _addr;
    socket_t _socket;
    std::atomic<socket_status> _status = socket_status::connected;
public:
    client(socket_t socket, sockaddr_in addr);
    ~client() override;

    socket_status disconnect() override;
    [[nodiscard]] socket_status status() const override { return _status; }
    [[nodiscard]] bool send_data(const std::vector<uint8_t> &buffer) const override;
    std::vector<uint8_t> receive_data() override;
    [[nodiscard]] ip_address address() const override {
        return {_addr.sin_addr.s_addr, _addr.sin_port};
    }
    [[nodiscard]] uint32_t host() const override { return _addr.sin_addr.s_addr; }
    [[nodiscard]] uint16_t port() const override { return _addr.sin_port; }
};

#endif //SEQ_GENERATOR_CLIENT_HXX
