//
// Created by MrMam on 12.09.2022.
//

#ifndef SEQ_GENERATOR_SERVER_HXX
#define SEQ_GENERATOR_SERVER_HXX

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <shared_mutex>
#include <map>
#include "client.hxx"
#include <seq_storage/seq_storage.hxx>

struct keep_alive_config {
    keep_alive_prop_t ka_idle = 120;
    keep_alive_prop_t ka_intvl = 3;
    keep_alive_prop_t ka_cnt = 5;
};

class server {
public:
    typedef std::function<void(std::vector<uint8_t>, client &)> handler_function_t;
    typedef std::function<void(client &)> con_handler_function_t;
    static constexpr auto default_data_handler = [](const std::vector<uint8_t> &, client &) {};
    static constexpr auto default_connection_handler = [](client &) {};

    enum class status_t : uint8_t {
        up = 0,
        err_socket_init = 1,
        err_socket_bind = 2,
        err_socket_keep_alive = 3,
        err_socket_listening = 4,
        close = 5,
        initializing = 6
    };

private:
    socket_t _serv_socket;
    uint16_t _port;
    status_t _status = status_t::close;
    handler_function_t _handler = default_data_handler;
    con_handler_function_t _connect_h = default_connection_handler;

    std::shared_ptr<task_handler> _task_h;
    keep_alive_config _ka_config;
    std::map<ip_address, std::unique_ptr<client>> _clients;
    mutable std::shared_mutex _clients_mutex;

    /// \brief Установка параметров сокета
    /// \param socket Сокет
    bool enable_keep_alive(socket_t socket);

    /// \brief Цикл - обработчик подключений
    void handling_accept_loop();

    /// \brief Обработчик подключения
    void handle_client(const ip_address &host);
    server() = default;
public:
    explicit server(uint16_t port,
           size_t thread_count = std::thread::hardware_concurrency());
    ~server() = default;
    server(const server&) = delete;
    server(server&&) = delete;
    server& operator=(const server&) = delete;
    server& operator=(server&&) = delete;

    [[nodiscard]] uint16_t port() const { return _port; }

    /// \brief Запуск сервера
    status_t start();
    void stop();

    /// \brief Добавление клиента в список
    /// \param client Клиент
    inline void remove_client(const ip_address &host);

    /// \brief Удаление клиента из списка
    /// \param client Клиент
    inline void add_client(std::unique_ptr<client> &&client);

    static std::string to_string(const ip_address &host) {
        uint32_t ip = host.first;
        return std::string() + std::to_string(int(reinterpret_cast<char *>(&ip)[0])) + '.' +
               std::to_string(int(reinterpret_cast<char *>(&ip)[1])) + '.' +
               std::to_string(int(reinterpret_cast<char *>(&ip)[2])) + '.' +
               std::to_string(int(reinterpret_cast<char *>(&ip)[3])) + ':' +
               std::to_string(host.second);
    }
};


#endif //SEQ_GENERATOR_SERVER_HXX
