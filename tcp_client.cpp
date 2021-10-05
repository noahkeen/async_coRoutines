#include "common.h"
#include "TcpClient.h"

int main(int argc, char *argv[]) {
    try {

        auto config = get_tcp_client_connection_info(argc, argv);

        // Create an io_context with a single thread
        asio::io_context io_context(1);
        // standard catch of SIGINT, SIGTERM signals for clean context destruction and shutdown
        asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) { io_context.stop(); });

        TcpClient client(io_context, config.ip, config.port, config.bind_address);

        // Every 5 secs send a msgs from another thread to server
        std::thread sendingThread([&] {
            while (true) {
                std::this_thread::sleep_for(std::chrono::seconds (5));
                client.send("ClientHeartbeat");
            }
        });

        // Start the IO context. A blocking call that has been wired to respond asynchonously with co_routines
        io_context.run();
    }
    catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}