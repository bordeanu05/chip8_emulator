cmake_minimum_required(VERSION 3.0)

project(chip8 LANGUAGES CXX) 

find_package(SDL2 REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc ${SDL2_INCLUDE_DIRS})

file(GLOB source_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)

add_executable(${PROJECT_NAME} ${source_files})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
