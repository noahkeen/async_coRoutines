#pragma once

#include "common.h"
#include "CoRoutineSocketSenderAndReceiver.h"

/**
 * A connected socket is used to construct TcpServerConnection.
 * The socket should be connected before construction - typically via a TcpAcceptor
 */
class TcpServerConnection
        : public CoRoutineSocketSenderAndReceiver<tcp::socket, FifoCircularMessageBuffer<std::string_view>> {
public:
    TcpServerConnection(tcp::socket socket, MessageCallback onMsgCallback = nullptr)
            : CoRoutineSocketSenderAndReceiver(std::move(socket), std::move(onMsgCallback)) {
        std::cout << "New TcpConnection from: " << socket_.remote_endpoint()
                  << " connected at: " << socket_.local_endpoint() << std::endl;
        start();
    }

private:
    awaitable<size_t> do_read(const asio::mutable_buffer &msg) {
        return socket_.async_read_some(msg, use_awaitable);
    };

    awaitable<size_t> do_write(const asio::const_buffer &msg) {
        return asio::async_write(socket_, msg, use_awaitable);
    };

};

/**
 * Accepts new connections at specified ip_address on specified port
 *
 * Note - does not establish outgoing connections - only accepts
 * If you need to connect as well see TcpClient
 */
class TcpServer {
public:
    explicit TcpServer(asio::io_context &io_context,
                       const std::string &ip_address,
                       uint16_t port,
                       MessageCallback messageCallback = nullptr) :
            messageCallback(std::move(messageCallback)) {
        std::cout << "Accepting new connections at " << ip_address << ":" << std::to_string(port) << std::endl;
        co_spawn(io_context,
                 create_listener(tcp::acceptor(io_context,
                                               create_endpoint<asio::ip::tcp::endpoint>(ip_address, port))),
                 detached);
    }

    void send_to_all(const std::string_view &msg) {
        for (auto &&connection : connections) {
            connection->send(msg);
        }
    }

private:
    MessageCallback messageCallback;
    std::vector<std::shared_ptr<TcpServerConnection>> connections;

    awaitable<void>
    create_listener(tcp::acceptor acceptor) {
        for (;;) {
            const std::shared_ptr<TcpServerConnection> &serverPtr = std::make_shared<TcpServerConnection>(
                    co_await acceptor.async_accept(use_awaitable),
                    messageCallback
            );
            connections.push_back(serverPtr);
        }
    }


};