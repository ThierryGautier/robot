cmake_minimum_required(VERSION 3.0)

project(voice_control LANGUAGES C)

include_directories(./../std/inc)
include_directories(./../supervisor/inc)
include_directories(./inc)

add_executable(voice_control ./src/voice_control.c )

target_link_libraries(voice_control)

install(TARGETS voice_control RUNTIME DESTINATION bin)
