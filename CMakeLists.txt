cmake_minimum_required(VERSION 3.10)
project(RestfulApi)

set(CMAKE_CXX_STANDARD 17)
set(MYCOMPILE_FLAGS)

find_package(Boost REQUIRED COMPONENTS system filesystem)
# find_package(cpp-jwt REQUIRED)
add_subdirectory(libs/cpp-jwt)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED IMPORTED_TARGET glib-2.0)
pkg_search_module(RDKAFKA REQUIRED IMPORTED_TARGET rdkafka)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(include)

set(MODELS_SOURCES 
    src/models/profile.cpp     
    src/models/room.cpp
    src/models/task.cpp
    src/models/config.cpp
    src/models/timer.cpp
    src/models/invite.cpp
    src/models/status.cpp
)

set(SERVICES_SOURCES 
    src/services/JWT_manager.cpp
    src/services/file_parser.cpp
    src/services/data_base_manager.cpp  
)

set(KAFKA_SOURCES 
    src/kafka/producer.cpp
    src/kafka/common.cpp
)

set(SERVER_SOURCES
    src/server/http_connection.cpp
    src/server/request_handler.cpp
)

add_executable(RestfulApi 
    src/main.cpp
    ${MODELS_SOURCES}
    ${SERVICES_SOURCES}
    ${SERVER_SOURCES}
    ${KAFKA_SOURCES}
)

target_link_libraries(RestfulApi ${MYCOMPILE_FLAGS} ${Boost_LIBRARIES} cpp-jwt::cpp-jwt PkgConfig::GLIB PkgConfig::RDKAFKA)