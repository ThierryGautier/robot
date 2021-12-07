cmake_minimum_required(VERSION 3.0)

project(sound_listen LANGUAGES C)

include_directories(./../std/inc)
include_directories(./inc)

add_executable(sound_listen
               ./src/energy.c 
               ./src/envelope.c 
               ./src/file_wav.c 
               ./src/LMS.c 
               ./src/PassBandFilter.c 
               ./src/sound_capture.c
               ./src/sound_listen.c)

target_link_libraries(sound_listen m)
               
install(TARGETS sound_listen RUNTIME DESTINATION bin)
