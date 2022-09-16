//
// Created by MrMam on 12.09.2022.
//

#include <iostream>
#include <server/client.hxx>

client::client(socket_t socket, sockaddr_in addr) :
        _socket(socket), _addr(addr) {
    _status.store(socket_status::connected);
}

client::~client() {
    if (_socket == -1) return;
    shutdown(_socket, SHUT_RD);
    close(_socket);
}

socket_status client::disconnect() {
    _status.store(socket_status::disconnected);
    if (_socket != 1) return _status;
    shutdown(_socket, SHUT_RDWR);
    close(_socket);
    _socket = -1;
    return _status;
}

bool client::send_data(const std::vector<uint8_t> &buffer) const {
    if (_status != socket_status::connected) return false;

    if (::write(_socket, buffer.data(), buffer.size()) == -1) {
        return false;
    }
    return true;
}

std::vector<uint8_t> client::receive_data(uint timeout) {
    if (_status != socket_status::connected) return {};
    std::vector<uint8_t> buffer(_buffer_size);
    fd_set set;
    struct timeval timeout_s{};
    int rv;
    FD_ZERO(&set);
    FD_SET(_socket, &set);
    timeout_s.tv_sec = timeout;
    timeout_s.tv_usec = 0;
    char value{};
    while (value != '\n') {
        rv = select(_socket + 1, &set, nullptr, nullptr, &timeout_s);
        if (rv == -1) {
            std::cout << "select error" << std::endl;
            return {};
        } else if (rv == 0) {
            std::cout << "Client " << _addr.sin_port << " timeout" << std::endl;
            return {};
        } else {
            auto answer = ::read(_socket, &value, sizeof(value));
            if (answer == 0) {
                disconnect();
                return {};
            } else if (answer == -1) {
                int err;
                socklen_t len = sizeof(err);
                getsockopt(_socket, SOL_SOCKET, SO_ERROR, &err, &len);
                std::cout << "Error while reading from socket: " << std::strerror(err) << std::endl;
            }
            buffer.emplace_back(value);
        }
    }
    return buffer;
}