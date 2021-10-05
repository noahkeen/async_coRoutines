#pragma once

#include <thread>
#include <iostream>
#include <deque>
#include <utility>
#include <mutex>
#include <vector>

#include <asio/io_context.hpp>
#include <asio/strand.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read_until.hpp>
#include <asio/redirect_error.hpp>
#include <asio/signal_set.hpp>
#include <asio/steady_timer.hpp>
#include <asio/use_awaitable.hpp>
#include <asio/write.hpp>
#include <asio/detached.hpp>
#include <asio/co_spawn.hpp>
#include <asio/awaitable.hpp>
#include <asio.hpp>


using asio::ip::tcp;
using asio::ip::udp;
using asio::awaitable;
using asio::co_spawn;
using asio::detached;
using asio::redirect_error;
using asio::use_awaitable;

typedef std::function<void(const char*, size_t)> MessageCallback;

constexpr size_t READ_BUFFER_SIZE = 1024;

inline void on_msgs_received(const char *data, size_t len) {
    std::cout << "Received: " << std::string(data, len) << std::endl;
}


template<typename EndpointType>
EndpointType create_endpoint(const std::string& ip_address, uint16_t port) {
    asio::error_code ec;
    asio::ip::address addr = asio::ip::address::from_string(ip_address, ec);
    if (ec) {
        throw std::runtime_error("Unable to determine ip_address: " + ip_address);
    }
    return {addr, port};
}



struct TcpClientInfo {
    std::string ip;
    uint16_t port;
    std::string bind_address;

    TcpClientInfo(const std::string &ip, uint16_t port, const std::string &bindAddress) : ip(ip), port(port),
                                                                                          bind_address(bindAddress) {}

};

inline TcpClientInfo get_tcp_client_connection_info(int argc, char *argv[]) {
    if (argc < 2 || argc > 5) {
        throw std::runtime_error("Usage: tcp_client <ip_address: to connect to> <port: to connect to> <bind_address:default 0.0.0.0>\n");
    }

    std::string ip_address;
    std::string bind_address("0.0.0.0");
    uint16_t port = 30001;
    if (argc == 4) {
        ip_address = std::string(argv[1]);
        port = std::atol(argv[2]);
        bind_address = std::string(argv[3]);
    } else if (argc == 3) {
        ip_address = std::string(argv[1]);
        port = std::atol(argv[2]);
    } else {
        ip_address = std::string(argv[1]);
    }
    return {ip_address,  port, bind_address,};
}

struct TcpServerInfo {
    std::string ip;
    uint16_t port;

    TcpServerInfo(std::string ip, uint16_t port) : ip(std::move(ip)), port(port) {}

};

inline TcpServerInfo get_tcp_server_info(int argc, char *argv[]) {
    if (argc != 1 && argc != 3) {
        throw std::runtime_error("Usage: tcp_server <ip: default any/all> <port:default 5569>\n");
    }

    std::string ip_address;
    uint16_t port = 5569;
    if (argc == 3) {
        ip_address = std::string(argv[1]);
        port = std::atoi(argv[2]);
    }
    return {ip_address, port};
}

struct MulticastConnectionInfo {
    std::string multicast_group;
    std::string bind_address;
    uint64_t multicast_port;

    MulticastConnectionInfo(std::string multicastGroup, std::string bindAddress  = "0.0.0.0", uint64_t multicastPort = 30001)
            : multicast_group(std::move(multicastGroup)), bind_address(std::move(bindAddress)), multicast_port(multicastPort) {}

};

inline MulticastConnectionInfo get_multicast_connection_info(int argc, char *argv[]) {
    if (argc < 2 || argc > 5) {
        throw std::runtime_error("Usage: udp_server <multicast_group> <multicast_port:default 30001> <bind_address:default 0.0.0.0>\n");
    }

    std::string multicast_group;
    std::string bind_address("0.0.0.0");
    uint64_t multicast_port = 30001;
    if (argc == 4) {
        multicast_group = std::string(argv[1]);
        multicast_port = std::atol(argv[2]);
        bind_address = std::string(argv[3]);
    } else if (argc == 3) {
        multicast_group = std::string(argv[1]);
        multicast_port = std::atol(argv[2]);
    } else {
        multicast_group = std::string(argv[1]);
    }
    return {multicast_group, bind_address, multicast_port};
}