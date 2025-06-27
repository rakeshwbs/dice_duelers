cmake_minimum_required(VERSION 3.10)
project(DiceDuelers)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

# Server Executable
add_executable(ServerApp
        main/server_main.cpp
        src/Server.cpp
)

# Client Executable
add_executable(ClientApp
        main/client_main.cpp
        src/Client.cpp
)

# Link Winsock2 on Windows
if (WIN32)
    target_link_libraries(ServerApp ws2_32)
    target_link_libraries(ClientApp ws2_32)
endif()
