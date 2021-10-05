#pragma once

#include "common.h"
#include "FifoCircularMessageBuffer.h"

template<typename SocketType, typename MessageQueueType>
class CoRoutineSocketSenderAndReceiver {
public:
    CoRoutineSocketSenderAndReceiver(SocketType socket, MessageCallback on_msg_callback = nullptr) :
            socket_(std::move(socket)),
            timer_(socket_.get_executor()),
            on_received_message_callback_(std::move(on_msg_callback)) {
        if (!on_received_message_callback_) {
            // No op - instead of conditional null check every msg - small perf improvement
            this->on_received_message_callback_ = [&](const char *, size_t) {};
        }
        timer_.expires_at(std::chrono::steady_clock::time_point::max());
    }

    void send(const std::string_view &msg) {
        if (msgs_to_send_.push(msg)) {
            timer_.cancel_one(); // signal writer co_routine waiting on timer
        } else {
            std::cerr << "Write Buffer Full. Can not push MSG. : " << msg << std::endl;
        }
    }

protected:
    SocketType socket_;

    /**
     * Socket async read implementation.
     *
     * E.g.: TCP :         return socket_.async_read_some(msg, use_awaitable);
     * E.g.: UDP :         return socket_.async_receive_from(msg, receiving_endpoint_, use_awaitable);
     *
     * @param msg
     * @return
     */
    virtual awaitable<size_t> do_read(const asio::mutable_buffer &msg) = 0;

    /**
     * Socket async write implementation.
     *
     * E.g.: TCP :         return asio::async_write(socket_, msg, use_awaitable);
     * E.g.: UDP :         return socket_.async_send_to(asio::buffer(msg), multicast_endpoint_, use_awaitable);
     *
     * @param msg
     * @return
     */
    virtual awaitable<size_t> do_write(const asio::const_buffer &msg) = 0;

    /**
     * Virtual in case you want to override and only read or only write
     */
    virtual void start() {
        // Start a co_routine to read
        co_spawn(socket_.get_executor(),
                 [&] { return reader(); },
                 detached);

        // Start a co_routine to write
        co_spawn(socket_.get_executor(),
                 [&] { return writer(); },
                 detached);

    }

    void stop() {
        timer_.cancel();
        socket_.close();
    }

private:
    asio::steady_timer timer_; // used for synchronization and signaling
    MessageCallback on_received_message_callback_;
    MessageQueueType msgs_to_send_;

    awaitable<void> reader() {
        try {
            for (char data[READ_BUFFER_SIZE] = {};;) {
                // co_await
                // wake up signal: when bytes are available on the socket
                std::size_t n = co_await do_read(asio::buffer(data));
                std::cout << "Read  " << n << std::endl;
                on_received_message_callback_(data, n);
            }
        } catch (std::exception &e) {
            std::cerr << "Exception while reading: " << e.what() << std::endl;
            stop();
        }
    }

    awaitable<void> writer() {
        try {
            while (socket_.is_open()) {
                std::string_view msg;
                if (msgs_to_send_.pop(msg)) {
                    // co_wait
                    // write msg using the virtual do_write child implementation.
                    co_await do_write(asio::buffer(msg));
                    std::cout << "Wrote: " << msg << std::endl;
                } else {
                    // co_wait
                    // wake up signal:
                    //      either the timer_.cancel_one() call above signaling a new msgs is available
                    //      or the timer's expiration
                    asio::error_code ec;
                    co_await timer_.async_wait(redirect_error(use_awaitable, ec));
                }
            }
        } catch (std::exception &e) {
            std::cerr << "Exception while writing: " << e.what() << std::endl;
            stop();
        }
    }



};