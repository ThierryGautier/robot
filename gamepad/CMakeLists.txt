cmake_minimum_required(VERSION 3.0)

project(gamepad LANGUAGES C)

include_directories(./../std/inc)
include_directories(./../supervisor/inc)
include_directories(./inc)

add_executable(gamepad ./src/gamepad.c)

target_link_libraries(gamepad rt)

install(TARGETS gamepad RUNTIME DESTINATION bin)
