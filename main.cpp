//
// Created by MrMam on 12.09.2022.
//
#include <csignal>
#include <iostream>
#include <server/server.hxx>

int main(int argc, char** argv) {
    if (argc == 1) {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
    }
    short port = strtol(argv[1], nullptr, 10);
    auto srv = server(port, std::thread::hardware_concurrency());
    try {
        auto status = srv.start();
            switch (status) {
                case server::status_t::initializing:
                case server::status_t::up:
                    std::cout << "Server is up on port: " << srv.port() << std::endl;
                    break;
                case server::status_t::err_socket_init:
                    std::cout << "Error: socket init" << std::endl;
                    break;
                case server::status_t::err_socket_bind:
                    std::cout << "Error: socket bind" << std::endl;
                    break;
                case server::status_t::err_socket_listening:
                    std::cout << "Error: socket listen" << std::endl;
                    break;
                case server::status_t::err_socket_keep_alive:
                    std::cout << "Error: socket keep alive" << std::endl;
                default:
                    std::cout << "Error: unknown" << std::endl;
                    break;
            }
        return 0;
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}

//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <cstdlib>
//#include <cstdio>
//#include <cstring>
//
//#define MY_SOCK_PATH "/somepath"
//#define LISTEN_BACKLOG 50
//
//#define handle_error(msg) \
//           do { perror(msg); exit(EXIT_FAILURE); } while (0)
//
//int
//main(int argc, char *argv[])
//{
//    int sfd, cfd;
//    struct sockaddr_in addr{}, peer_addr;
//    socklen_t peer_addr_size;
//
//    sfd = socket(AF_INET, SOCK_STREAM, 0);
//    if (sfd == -1)
//        handle_error("socket");
//
//    addr.sin_addr.s_addr = INADDR_ANY;
//    addr.sin_port = htons(32451);
//    addr.sin_family = AF_INET;
//
//    if (bind(sfd, (struct sockaddr *) &addr,
//             sizeof(addr)) == -1)
//        handle_error("bind");
//
//    if (listen(sfd, LISTEN_BACKLOG) == -1)
//        handle_error("listen");
//
//    /* Now we can accept incoming connections one
//       at a time using accept(2) */
//
//    peer_addr_size = sizeof(struct sockaddr_in);
//    cfd = accept(sfd, (struct sockaddr *) &peer_addr,
//                 &peer_addr_size);
//    if (cfd == -1)
//        handle_error("accept");
//
//    /* Code to deal with incoming connection(s)... */
//
//    /* When no longer required, the socket pathname, MY_SOCK_PATH
//       should be deleted using unlink(2) or remove(3) */
//}