cmake_minimum_required(VERSION 3.10)
project(server)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-O2 -Wall -g")

file(GLOB LOG_SOURCES ../code/log/*.cpp)
file(GLOB POOL_SOURCES ../code/pool/*.cpp)
file(GLOB TIMER_SOURCES ../code/timer/*.cpp)
file(GLOB HTTP_SOURCES ../code/http/*.cpp)
file(GLOB SERVER_SOURCES ../code/server/*.cpp)
file(GLOB BUFFER_SOURCES ../code/buffer/*.cpp)
set(MAIN_SOURCE ../code/main.cpp)

set(SOURCES
    ${LOG_SOURCES}
    ${POOL_SOURCES}
    ${TIMER_SOURCES}
    ${HTTP_SOURCES}
    ${SERVER_SOURCES}
    ${BUFFER_SOURCES}
    ${MAIN_SOURCE}
)

add_executable(server ${SOURCES})
target_link_libraries(server pthread mysqlclient)
