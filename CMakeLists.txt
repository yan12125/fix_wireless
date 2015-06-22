cmake_minimum_required(VERSION 2.8.11)

project(fix_wireless)

find_package(Qt5Core)
find_package(Qt5DBus)
find_package(PkgConfig)
pkg_check_modules(LIBUSB REQUIRED libusb)

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++11 ${LIBUSB_CFLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${LIBUSB_LDFLAGS}")

add_executable(fix_wireless fix_wireless.cpp)

target_link_libraries(fix_wireless Qt5::Core Qt5::DBus)
