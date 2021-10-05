#include "common.h"
#include "UdpClient.h"


int main(int , char *[]) {
    try {


        // Create an io_context with a single thread
        asio::io_context io_context(1);
        // standard catch of SIGINT, SIGTERM signals for clean context destruction and shutdown
        asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) { io_context.stop(); });

        UdpClient udpClient(io_context, "239.255.0.1", 5556, "10.0.0.83");
        std::thread sendingThread([&] {
            while (true) {
                std::this_thread::sleep_for(std::chrono::seconds (2));
                udpClient.send("ClientPing");
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