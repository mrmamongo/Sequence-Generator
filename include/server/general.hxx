//
// Created by MrMam on 12.09.2022.
//

#ifndef SEQ_GENERATOR_GENERAL_HXX
#define SEQ_GENERATOR_GENERAL_HXX

#include <utils/constants.hxx>
#include <utils/task_handler.hxx>

#include <sys/socket.h>

#include <cstdint>
#include <cstring>
#include <cinttypes>
#include <malloc.h>

#include <queue>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

typedef int socket_t;
typedef int keep_alive_prop_t;

constexpr uint32_t LOCALHOST_IP = 0x0100007f;

enum class socket_status : uint8_t {
  connected = 0,
  err_socket_init = 1,
  err_socket_bind = 2,
  err_socket_connect = 3,
  disconnected = 4
};

class client_base {
public:
    virtual ~client_base() {};
    virtual socket_status disconnect() = 0;
    virtual socket_status status() const = 0;
    virtual bool send_data(const std::vector<uint8_t>&) const = 0;
    virtual std::vector<uint8_t> receive_data() = 0;
    virtual uint32_t host() const = 0;
    virtual uint16_t port() const = 0;
    virtual ip_address address() const = 0;
};

#endif //SEQ_GENERATOR_GENERAL_HXX
