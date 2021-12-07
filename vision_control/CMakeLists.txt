cmake_minimum_required(VERSION 3.0)

project(vision_control LANGUAGES CXX)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories(./../std/inc)
include_directories(./../supervisor/inc)
include_directories(./inc)

add_executable(vision_control ./src/vision_control.cpp ./src/com.cpp )

target_link_libraries(vision_control
                      ${OpenCV_LIBS} 
                      pthread 
                      rt)

install(TARGETS vision_control RUNTIME DESTINATION bin)

