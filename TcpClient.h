#pragma once

#include "common.h"
#include "CoRoutineSocketSenderAndReceiver.h"

/**
 * Makes a connection when instantiated to the specified endpoint
 */
class TcpClient : public CoRoutineSocketSenderAndReceiver<tcp::socket, FifoCircularMessageBuffer<std::string_view>> {
public:
    TcpClient(asio::io_context &io_context,
              const std::string &ip_address,
              uint16_t port,
              const std::string &bind_address = "0.0.0.0",
              MessageCallback onMsgCallback = nullptr)
            : CoRoutineSocketSenderAndReceiver(tcp::socket(io_context), std::move(onMsgCallback)) {
        connect(ip_address, port, bind_address);
    }

private:

    awaitable <size_t> do_read(const asio::mutable_buffer &msg) override {
        return socket_.async_read_some(msg, use_awaitable);
    };

   awaitable <size_t> do_write(const asio::const_buffer &msg) override {
        return asio::async_write(socket_, msg, use_awaitable);
    };

    void connect(const std::string &ip_address,
                 uint16_t port,
                 const std::string &bind_address) {
        try {
            // open socket and set options
            auto connect_endpoint = create_endpoint<asio::ip::tcp::endpoint>(ip_address, port);
            auto bind_endpoint = create_endpoint<asio::ip::tcp::endpoint>(bind_address, 0);
            std::cout << "Attempting TcpConnection to: " << connect_endpoint
                      << " from : " << bind_endpoint << std::endl;

            socket_.open(tcp::v4());
            socket_.set_option(tcp::socket::reuse_address(true));
            socket_.set_option(tcp::no_delay(true));
            socket_.non_blocking(true);
            // bind
            socket_.bind(bind_endpoint);
            // async connect
            socket_.async_connect(connect_endpoint, [&](const asio::error_code &error) {
                if (!error) {
                    start();
                } else {
                    std::cerr << "Error connecting Socket : " << error << std::endl;
                    stop();
                }
            });
        } catch (std::exception &e) {
            std::cerr << "Exception while Connecting Socket: " << e.what() << std::endl;
            stop();
        }
    }
};
