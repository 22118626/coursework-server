cmake_minimum_required(VERSION 3.27)
project(coursework-server)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)
# include OpenSSL
set(OPENSSL_ROOT_DIR "${CMAKE_SOURCE_DIR}/OpenSSL-Win64")
set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")

set(OPENSSL_USE_STATIC_LIBS TRUE)

if (CMAKE_BUILD_TYPE STREQUAL "Debug") #static libraries or static debug depending on build type
    set(OPENSSL_LIB_DIR "${OPENSSL_ROOT_DIR}/lib/VC/x64/MTd")
else ()
    set(OPENSSL_LIB_DIR "${OPENSSL_ROOT_DIR}/lib/VC/x64/MT")
endif ()

set(OPENSSL_CRYPTO_LIBRARY "${OPENSSL_LIB_DIR}/libcrypto.lib")
set(OPENSSL_SSL_LIBRARY "${OPENSSL_LIB_DIR}/libssl.lib")

# end OpenSSL Definitions
find_program(MINGW_GCC_EXECUTABLE NAMES mingw32-gcc gcc)
if(MINGW_GCC_EXECUTABLE)
    get_filename_component(MINGW_BIN_PATH ${MINGW_GCC_EXECUTABLE} DIRECTORY)
    get_filename_component(MINGW_ROOT_PATH ${MINGW_BIN_PATH} DIRECTORY)
    message(STATUS "MinGW root path is: ${MINGW_ROOT_PATH}")
    link_directories(${MINGW_ROOT_PATH}/bin)
    include_directories(${MINGW_ROOT_PATH}/include)
    #target_link_libraries(your_target PRIVATE winpthread)
endif ()

# Force static linking of winpthread
if(MINGW)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)

    # Ensure static libraries are used
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc")

endif()


file(GLOB SRC_FILES src/*.cpp)

include_directories(${OPENSSL_INCLUDE_DIR})

fetchcontent_declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

#include_directories(${OPENSSL_INCLUDE_DIR})
set(OPENSSL_LIBRARIES "${OPENSSL_SSL_LIBRARY};${OPENSSL_CRYPTO_LIBRARY}")



add_executable(coursework-server ${SRC_FILES})

file(COPY "${CMAKE_SOURCE_DIR}/OpenSSL-Win64" DESTINATION ${CMAKE_BINARY_DIR})

#[=[find_package(OpenSSL REQUIRED)  // this line was overriding the previously defined OpenSSL directories which led cmake to have a brain fart]=]

#link
target_link_libraries(${PROJECT_NAME} PRIVATE
        ws2_32
        ${OPENSSL_LIBRARIES}
        nlohmann_json::nlohmann_json
        Threads::Threads
        #"D:/Program Files/CLion 2024.2.2/bin/mingw/x86_64-w64-mingw32/lib/libwinpthread.a"
)


