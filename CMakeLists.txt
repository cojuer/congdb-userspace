cmake_minimum_required(VERSION 2.8)
project(congdb-ctl C CXX)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
set(CMAKE_CXX_STANDARD 11)

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_MODULE_PATH})
find_package(Libnl REQUIRED)

include_directories(BEFORE ${LIBNL_INCLUDE_DIRS})

set(CTL_SOURCES congdb_ctl.cpp)

add_executable(congdb-ctl ${CTL_SOURCES})

target_link_libraries(congdb-ctl congdb-api ${LIBNL_LIBRARIES})

install (TARGETS congdb-ctl DESTINATION /usr/local/bin)
