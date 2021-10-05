#pragma once

#include "common.h"
#include "FifoCircularMessageBuffer.h"
#include "CoRoutineSocketSenderAndReceiver.h"

class UdpClient :  public CoRoutineSocketSenderAndReceiver<udp::socket, FifoCircularMessageBuffer<std::string_view>> {
    udp::endpoint receiving_endpoint_;
    udp::endpoint multicast_endpoint_;
public:
    UdpClient(asio::io_context &io_context,
              const std::string &multicast_group,
              uint16_t multicast_port,
              const std::string &bind_address = "0.0.0.0") : // bind_address = "0.0.0.0" = all
            CoRoutineSocketSenderAndReceiver(udp::socket(io_context)),
            receiving_endpoint_(create_endpoint<udp::endpoint>(bind_address, 0)),  // port = 0 = any port
            multicast_endpoint_(create_endpoint<udp::endpoint>(multicast_group, multicast_port)) {
        std::cout << "Will join multicast group: " << multicast_endpoint_
                  << " and listen from: " << receiving_endpoint_ << std::endl;
        connect(multicast_group, bind_address);
    }

protected:

    /**
     * We read from receiving_endpoint_
     * We have joined and bound to multicast_endpoint_
     * @param msg
     * @return
     */
    awaitable <size_t> do_read(const asio::mutable_buffer &msg) override {
        return socket_.async_receive_from(msg, receiving_endpoint_, use_awaitable);
    };

    /**
     * We send to multicast_endpoint_
     * @param msg
     * @return
     */
    awaitable <size_t> do_write(const asio::const_buffer &msg) override {
        return socket_.async_send_to(asio::buffer(msg), multicast_endpoint_, use_awaitable);
    };

private:

    void connect(const std::string &multicast_group,
                 const std::string &bind_address) {

        // create addresses from strings
        asio::error_code ec;
        asio::ip::address bind_addr = asio::ip::address::from_string(bind_address, ec);
        if (ec) {
            throw std::runtime_error("Unable to determine bind_address: " + bind_address);
        }
        asio::ip::address mcast_addr = asio::ip::address::from_string(multicast_group, ec);
        if (ec) {
            throw std::runtime_error("Unable to determine multicast_group: " + multicast_group);
        }

        // open socket for UDP
        socket_.open(udp::v4());

        // set options
        socket_.set_option(asio::ip::udp::socket::reuse_address(true));
        socket_.set_option(asio::ip::multicast::enable_loopback(true));
        int pollInterval = 50; //value recommended in the RHEL tuning guide
        setsockopt(socket_.native_handle(), SOL_SOCKET, SO_BUSY_POLL, &pollInterval, sizeof(int));

        // bind to multicast_endpoint_
        socket_.bind(multicast_endpoint_);

        // join the multicast group:port on the specified bind_address
        socket_.set_option(asio::ip::multicast::join_group(mcast_addr.to_v4(), bind_addr.to_v4()), ec);
        if (ec) {
            throw std::runtime_error("Unable to join : " + multicast_group + " on : " + bind_address);
        }

        start();
    }

};
