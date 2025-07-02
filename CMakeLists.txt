cmake_minimum_required(VERSION 3.10)
project(DiceDuelers)

set(CMAKE_CXX_STANDARD 17)

# Include header files
include_directories(include)

# Common source files shared by all targets
set(COMMON_SOURCES
        src/Player.cpp
        src/Dice.cpp
        src/GameManager.cpp
        src/NetworkManager.cpp
)


# Server executable (with networking)
add_executable(ServerApp
        main/server_main.cpp
        src/Server.cpp
        ${COMMON_SOURCES}
)

# Client executable (with networking)
add_executable(ClientApp
        main/client_main.cpp
        src/Client.cpp
        ${COMMON_SOURCES}
)

# Link Winsock2 on Windows
if (WIN32)
    target_link_libraries(ServerApp ws2_32)
    target_link_libraries(ClientApp ws2_32)
endif()
