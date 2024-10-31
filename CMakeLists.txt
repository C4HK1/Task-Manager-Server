cmake_minimum_required(VERSION 3.10)
project(RestfulApi)

set(CMAKE_CXX_STANDARD 17)
set(MYCOMPILE_FLAGS)

find_package(Boost REQUIRED COMPONENTS system filesystem)
# find_package(cpp-jwt REQUIRED)
add_subdirectory(./cpp-jwt)

include_directories(${Boost_INCLUDE_DIRS})

add_executable(RestfulApi 
    main.cpp
    models/profile/profile.h
    models/room/room.h
    models/task/task.h
    models/profile/profile.cpp
    models/room/room.cpp
    models/task/task.cpp
    models/config/config.h
    models/config/config.cpp
    models/timer/timer.h
    models/timer/timer.cpp
    services/JWT/JWT_manager.h
    services/JWT/JWT_manager.cpp
    models/global-status/global_status.h
    models/global-status/global_status.cpp
    server/connection/http_connection.h
    server/connection/http_connection.cpp
    server/request-handler/request_handler.h    
    server/request-handler/request_handler.cpp
    services/file-parser/file_parser.h
    services/file-parser/file_parser.cpp
    services/data-base/data_base_manager.h
    services/data-base/data_base_manager.cpp        
)

target_link_libraries(RestfulApi ${MYCOMPILE_FLAGS} ${Boost_LIBRARIES} cpp-jwt::cpp-jwt)