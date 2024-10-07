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
    models/Room/room.h
    models/Task/task.h
    models/Profile/profile.h
    models/Room/room.cpp
    models/Task/task.cpp
    models/Profile/profile.cpp
    services/JWT/JWT_manager.h
    services/JWT/JWT_manager.cpp
    "services/Server Connection/http_connection.h"
    "services/Server Connection/http_connection.cpp"
    "services/Text File Parser/text_file_parser.h"
    "services/Text File Parser/text_file_parser.cpp"
    "services/Data Base/data_base_manager.h"
    "services/Data Base/data_base_manager.cpp"    
)

target_link_libraries(RestfulApi ${MYCOMPILE_FLAGS} ${Boost_LIBRARIES} cpp-jwt::cpp-jwt)