cmake_minimum_required(VERSION 3.10)
project(DataBase)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS system filesystem)
add_subdirectory(cpp-jwt)

include_directories(${Boost_INCLUDE_DIRS})

add_executable(DataBase 
    main.cpp
    data_base_manager.h
    data_base_manager.cpp
)

target_link_libraries(DataBase ${Boost_LIBRARIES} cpp-jwt::cpp-jwt)