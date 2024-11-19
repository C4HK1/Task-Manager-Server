cmake_minimum_required(VERSION 3.10)
project(DataBase)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(cpp-jwt REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED IMPORTED_TARGET glib-2.0)
pkg_search_module(RDKAFKA REQUIRED IMPORTED_TARGET rdkafka)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(../../include)

add_executable(DataBase 
    main.cpp
    data_base_manager.cpp
    ../models/profile.cpp
    ../models/room.cpp
    ../models/profile.cpp
    ../models/room.cpp
    ../models/task.cpp
    ../models/config.cpp
    ../models/timer.cpp
    ../models/invite.cpp
    ../models/status.cpp
)

target_link_libraries(DataBase ${Boost_LIBRARIES} cpp-jwt::cpp-jwt PkgConfig::GLIB PkgConfig::RDKAFKA)