cmake_minimum_required(VERSION 3.5)

project (catbot)

include(FetchContent)

find_package(Threads REQUIRED)

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

option(DPP_BUILD_TEST "" OFF)
FetchContent_Declare(DPP GIT_REPOSITORY https://github.com/brainboxdotcc/DPP GIT_TAG master)
FetchContent_MakeAvailable(DPP)

FILE(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(catbot ${SOURCES})

target_link_libraries(catbot Threads::Threads OpenSSL::Crypto OpenSSL::SSL dpp)
