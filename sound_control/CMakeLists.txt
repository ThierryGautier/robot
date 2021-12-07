cmake_minimum_required(VERSION 3.0)

project(sound_listen LANGUAGES CXX)

include_directories(./../std/inc)
include_directories(./inc)

add_executable(sound_control
               ./src/sound_control.cpp)

target_link_libraries(sound_control m)
               
install(TARGETS sound_control RUNTIME DESTINATION bin)
