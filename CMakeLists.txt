cmake_minimum_required(VERSION 3.10)
project(RestfulApi)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(cpp-jwt REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

add_executable(RestfulApi 
    main.cpp
    services/JWT/JWT_manager.h
    services/JWT/JWT_manager.cpp
    "services/Server Connection/http_connection.h"
    "services/Server Connection/http_connection.cpp"
    "services/Text File Parser/text_file_parser.h"
    "services/Text File Parser/text_file_parser.cpp"
    "services/Data Base/data_base_manager.h"
    "services/Data Base/data_base_manager.cpp"    
)

target_link_libraries(RestfulApi ${Boost_LIBRARIES} cpp-jwt::cpp-jwt)