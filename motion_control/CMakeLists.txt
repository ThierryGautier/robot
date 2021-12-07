cmake_minimum_required(VERSION 3.0)

project(motion_control LANGUAGES C)

include_directories(./../std/inc)
include_directories(./../supervisor/inc)
include_directories(./inc)

add_executable(motion_control ./src/motion_control.c ./src/hdlc.c ./src/uart.c )

target_link_libraries(motion_control pthread rt)

install(TARGETS motion_control RUNTIME DESTINATION bin)
