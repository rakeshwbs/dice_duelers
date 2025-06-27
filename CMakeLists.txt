cmake_minimum_required(VERSION 3.10)
project(DiceDuelers)

# Set C++ version
set(CMAKE_CXX_STANDARD 17)

# Include header files from include/ folder
include_directories(include)

# Source files
set(SOURCES
        main/main.cpp
        src/Server.cpp
        src/Client.cpp
)

# Executable target
add_executable(DiceDuelers ${SOURCES})

# Link Winsock2 library (Windows only)
if (WIN32)
    target_link_libraries(DiceDuelers ws2_32)
endif()
