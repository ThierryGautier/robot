cmake_minimum_required(VERSION 3.0)

project(supervisor LANGUAGES C)

include_directories(./../std/inc)
include_directories(./../gamepad/inc)
include_directories(./../motion_control/inc)
include_directories(./../vision_control/inc)
include_directories(./inc)

add_executable(supervisor ./src/supervisor.c)

target_link_libraries(supervisor m rt)

install(TARGETS supervisor RUNTIME DESTINATION bin)
