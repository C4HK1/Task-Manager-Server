cmake_minimum_required(VERSION 3.10)
project(DataBase)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(cpp-jwt REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

add_executable(DataBase 
    main.cpp
    data_base_manager.h
    data_base_manager.cpp
    ../../models/profile/profile.h
    ../../models/room/room.h
    ../../models/task/task.h
    ../../models/profile/profile.cpp
    ../../models/room/room.cpp
    ../../models/task/task.cpp
    ../../models/config/config.h
    ../../models/config/config.cpp
    ../../models/timer/timer.cpp
    ../../models/timer/timer.cpp
)

target_link_libraries(DataBase ${Boost_LIBRARIES} cpp-jwt::cpp-jwt)