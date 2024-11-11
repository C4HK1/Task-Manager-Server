cmake_minimum_required(VERSION 3.10)
project(DataBase)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(cpp-jwt REQUIRED)

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
)

target_link_libraries(DataBase ${Boost_LIBRARIES} cpp-jwt::cpp-jwt)