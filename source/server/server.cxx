//
// Created by MrMam on 12.09.2022.
//

#include <utility>
#include <iostream>

#include <server/server.hxx>

server::server(const uint16_t port,
               size_t thread_count) :
        _port(port),
        _task_h(new task_handler(thread_count)) {}

server::status_t server::start() {
    _task_h->execute([this] { handling_accept_loop(); });
    _status = (status_t::initializing);
    return _status;
}

void server::stop() {
    _status = (status_t::close);
    close(_serv_socket);
    _clients.clear();
    std::cout << "Server stopped" << std::endl;
}

bool server::enable_keep_alive(socket_t socket) {
    int flag = 1;
    int on = 1;
    if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        return false;
    }
    if (setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(flag)) == -1) return false;
    if (setsockopt(socket, IPPROTO_TCP, TCP_KEEPIDLE, &_ka_config.ka_idle, sizeof(_ka_config.ka_idle)) == -1)
        return false;
    if (setsockopt(socket, IPPROTO_TCP, TCP_KEEPINTVL, &_ka_config.ka_intvl, sizeof(_ka_config.ka_intvl)) == -1)
        return false;
    if (setsockopt(socket, IPPROTO_TCP, TCP_KEEPCNT, &_ka_config.ka_cnt, sizeof(_ka_config.ka_cnt)) == -1) return false;
    return true;
}

void server::handling_accept_loop() {
    for (;;) {
        if (_status != status_t::up) {
            sockaddr_in addr{};
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
            addr.sin_port = htons(_port);
            addr.sin_family = AF_INET;

            if (_serv_socket = socket(AF_INET, SOCK_STREAM, 0); _serv_socket == -1) {
                std::cout << "Server socket init error" << std::endl;
                stop();
                _task_h->stop();
                return;
            }

            if (bind(_serv_socket, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
                std::cout << "Server socket bind error" << std::endl;
                stop();
                _task_h->stop();
                return;
            }
            if (listen(_serv_socket, SOMAXCONN) == -1) {
                std::cout << "Server socket listen error" << std::endl;
                stop();
                _task_h->stop();
                return;
            }

            if (!enable_keep_alive(_serv_socket)) {
                std::cout << "Server socket keep alive error" << std::endl;
                _task_h->stop();
                return;
            }
            _status = status_t::up;
        } else {
            socklen_t addr_size = sizeof(sockaddr_in);
            sockaddr_in addr{};
            socket_t client_socket;
            if (client_socket =
                        (accept(_serv_socket, (struct sockaddr *) &addr, &addr_size)); client_socket == -1) {
                continue;
            }
            std::cout << "Client connected: " << (addr.sin_port) << client_socket << std::endl;
            if (!enable_keep_alive(client_socket)) {
                shutdown(client_socket, SHUT_RDWR);
                close(client_socket);
                std::cerr << "Keep alive error" << std::endl;
                return;
            }

            std::unique_ptr<client> client_ptr = std::make_unique<client>(client_socket, addr);
            auto address = client_ptr->address();

            add_client(std::move(client_ptr));

            _task_h->execute([address, this] {
                handle_client(address);
            });
        }
    }
}

void server::remove_client(const ip_address &host) {
    _clients_mutex.lock();
    try {
        _clients.erase(host);
    } catch (...) {
    }
    _clients_mutex.unlock();
    seq_storage::instance()->remove_sequence(host);
}

void server::handle_client(const ip_address &addr) {
    std::unique_ptr<client> *client_ptr;
    _clients_mutex.lock_shared();
    client_ptr = &_clients[addr];
    _clients_mutex.unlock_shared();
    std::cout << "Connection " << to_string((*client_ptr)->address()) << " accepted\n";
    std::vector<uint8_t> received{};
    std::string query{};
    do {
        received = (*client_ptr)->receive_data();
        query = {received.begin(), received.end()};
        if (!received.empty()) {
            if (query.find("export") != std::string::npos) {
                while (true) {
                    try {
                        if ((seq_storage::instance()->valid((*client_ptr)->address()))) {
                            auto data = seq_storage::instance()->next((*client_ptr)->address());
                            if (!(*client_ptr)->send_data(std::vector < uint8_t > {data.begin(), data.end()})) {
                                break;
                            }
                        } else {
                            (*client_ptr)->send_data(std::vector<uint8_t>{'E', 'R', 'R', 'O', 'R', '\n'});
                            break;
                        }
                    } catch (std::exception &e) {
                        std::cout << e.what() << std::endl;
                        break;
                    }
                }
            } else if (query.find("seq") != std::string::npos) {
                if (!seq_storage::instance()->contains((*client_ptr)->address()))
                    seq_storage::instance()->create_sequence((*client_ptr)->address());
                if (seq_storage::instance()->add_sequence((*client_ptr)->address(), query)) {
                    if (!(*client_ptr)->send_data(std::vector<uint8_t>{'O', 'K', '\n', '\r'})) {
                        break;
                    }
                } else {
                    if (!(*client_ptr)->send_data(std::vector<uint8_t>{'E', 'R', 'R', 'O', 'R', '\n', '\r'})) {
                        break;
                    }
                }
            } else {
                if (!(*client_ptr)->send_data(
                        std::vector<uint8_t>{'U', 'n', 'e', 'x', 'p', 'e', 'c', 't', 'e', 'd', ' ', 'c', 'o', 'm', 'm',
                                             'a', 'n', 'd', '\n', '\r'})){
                    break;
                }
            }
        } else {
            break;
        }
    } while (!received.empty());
    (*client_ptr)->disconnect();
    std::cout << "Connection " << to_string((*client_ptr)->address()) << " closed\n";
    remove_client((*client_ptr)->address());
}

void server::add_client(std::unique_ptr<client> &&client) {
    _clients_mutex.lock();
    try {
        _clients.emplace(client->address(), std::move(client));
    } catch (...) {
    }
    _clients_mutex.unlock();
}
