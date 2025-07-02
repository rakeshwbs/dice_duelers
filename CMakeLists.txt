cmake_minimum_required(VERSION 3.10)
project(DiceDuelers)

set(CMAKE_CXX_STANDARD 17)

# Include headers
include_directories(include)

# Common source files
set(COMMON_SOURCES
        src/Player.cpp
        src/Dice.cpp
        src/GameManager.cpp
        src/NetworkManager.cpp
)

# Server executable
add_executable(ServerApp
        main/server_main.cpp
        src/Server.cpp
        ${COMMON_SOURCES}
)

# Client executable
add_executable(ClientApp
        main/client_main.cpp
        src/Client.cpp
        ${COMMON_SOURCES}
)

# Link Winsock for Windows
if (WIN32)
    target_link_libraries(ServerApp ws2_32)
    target_link_libraries(ClientApp ws2_32)
endif()

# Optional: organize files in IDE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "Source Files" FILES ${COMMON_SOURCES})
