#pragma once

#include <sys/types.h>

enum DATA_BASE_EXECUTION_STATUS : u_int64_t;
enum JWT_EXECUTION_STATUS : u_int64_t;
enum FILE_PARSER_EXECUTION_STATUS : u_int64_t;
enum REQUEST_STATUS : u_int64_t {
    REQUEST_COMPLETED_SUCCESSFULY,
    
    REQUEST_INVALID_REQUEST_BODY_DATA,
};

constexpr size_t REQUEST_SHIFT{3};


struct server_status {
    static DATA_BASE_EXECUTION_STATUS data_base_status;
    static JWT_EXECUTION_STATUS JWT_status;
    static FILE_PARSER_EXECUTION_STATUS file_parse_status;
    static REQUEST_STATUS request_status;

    static auto get_status() -> int;
};