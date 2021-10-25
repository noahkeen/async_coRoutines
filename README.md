
# Async Networking with CoRoutines (C++20)

### There are 3 main classes: TCPClient, TCPServer, UDPClient

### TCPClient
    Connects to the specified IP and Port on an <optionally> specified bind interface
    Asyncronously reads from and writes to the socket
    Thread safe

### TCPServer
    Accepts new incoming TCP connections on the specified IP and Port on an <optionally> specified bind interface
    Asyncronously reads from and writes to each connected client
    Thread safe

### UDPClient
    Joins the specified multicast group:port on an <optionally> specified bind interface
    Asyncronously reads from and writes to the mulicast group:port
    Can be configured to only read or only write or both
    Thread safe

##### Notes:
    The FifoCircularMessageBuffer is used as the underlying 'lock-free' data structure to facilitate thread
    safe writing. It's used here as an example for simplicity. If you are looking to customize and optomize start here
    Also, for simplicity, std:err and std:out are used for logging.
    Most of the coroutine logic is in CoRoutineSocketSenderAndReceiver 
    Although you likely want to run them on separate machines they have been written to run on a single box

#### Dependencies:
    C++20 (gcc 10.3) with coroutines
    Asio (can use non-boost) : https://think-async.com/Asio/asio-1.18.2/
        Installed to : /usr/local/asio-1.18.2/include (or update CMakeLists.txt)
        Configured with C++17+, coroutines. Eg:
            /usr/local/asio-1.18.2/configure CXXFLAGS=" -std=c++2a -fcoroutines"
            make
            make install

#### Build:
    cmake3 .
    make -j5

#### Run:
    (these two will talk to each other)
    ./bin/tcp_server 192.128.0.27 5556 <optional bind interface(use a local nic): 10.0.0.1>
    ./bin/tcp_client 192.128.0.27 5556 <optional bind interface(use a local nic): 10.0.0.1>

    (start multiple to watch them converse)
    ./bin/udp_client 239.255.0.1 5599  <optional bind interface(use a local nic): 10.0.0.1>




