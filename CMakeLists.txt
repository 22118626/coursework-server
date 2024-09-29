cmake_minimum_required(VERSION 3.27)
project(coursework-server)

set(CMAKE_CXX_STANDARD 23)

file(GLOB SRC_FILES src/*.cpp)

add_executable(coursework-server ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
