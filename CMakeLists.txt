cmake_minimum_required(VERSION 3.27)
project(coursework-server)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)
# include OpenSSL
set(OPENSSL_ROOT_DIR "${CMAKE_SOURCE_DIR}/OpenSSL-Win64")
set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")

if (CMAKE_BUILD_TYPE STREQUAL "Debug") #static libraries or static debug depending on build type
    set(OPENSSL_LIB_DIR "${OPENSSL_ROOT_DIR}/lib/VC/x64/MDd")
else ()
    set(OPENSSL_LIB_DIR "${OPENSSL_ROOT_DIR}/lib/VC/x64/MD")
endif ()

set(OPENSSL_CRYPTO_LIBRARY "${OPENSSL_LIB_DIR}/libcrypto.lib")
set(OPENSSL_SSL_LIBRARY "${OPENSSL_LIB_DIR}/libssl.lib")
# end OpenSSL Definitions


file(GLOB SRC_FILES src/*.cpp)

include_directories(${OPENSSL_INCLUDE_DIR})

fetchcontent_declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

add_executable(coursework-server ${SRC_FILES})

set(OPENSSL_LIBRARIES "${OPENSSL_SSL_LIBRARY};${OPENSSL_CRYPTO_LIBRARY}")

file(COPY "${CMAKE_SOURCE_DIR}/OpenSSL-Win64" DESTINATION ${CMAKE_BINARY_DIR})

#[[find_package(OpenSSL REQUIRED)  // this line was overriding the previously defined OpenSSL directories which led cmake to have a brain fart]]

#link
target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 ${OPENSSL_LIBRARIES} nlohmann_json::nlohmann_json)


