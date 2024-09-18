cmake_minimum_required(VERSION 3.27)
project(coursework-server)

set(CMAKE_CXX_STANDARD 23)

add_executable(coursework-server src/main.cpp
        src/CLIApp.cpp
        src/Socket.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
