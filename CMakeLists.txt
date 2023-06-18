cmake_minimum_required(VERSION 3.5)

project (catbot)

find_package(Threads REQUIRED)

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

FILE(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(catbot ${SOURCES})

target_link_libraries(catbot Threads::Threads OpenSSL::Crypto OpenSSL::SSL)
