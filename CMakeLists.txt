# Specify the minimum required version of CMake
cmake_minimum_required(VERSION 3.14)

# Define the project name
project(BinanceKlineCollector)

# Set the C++ compiler to g++-13
set(CMAKE_CXX_COMPILER g++-13)

# Require C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define the executable target and its source files
add_executable(binacppwebsock
    main.cpp
    bina_cpp_websock.cpp
)

# Link the executable with required libraries
target_link_libraries(binacppwebsock
    PRIVATE
    ixwebsocket    # WebSocket library
    ssl            # OpenSSL SSL library
    crypto         # OpenSSL Crypto library
    z              # zlib compression library
    pthread        # POSIX threads
)
