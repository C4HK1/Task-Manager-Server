#pragma once

#include <sys/types.h>

enum DATA_BASE_EXECUTION_STATUS : u_int64_t;
enum JWT_EXECUTION_STATUS : u_int64_t;
enum FILE_PARSER_EXECUTION_STATUS : u_int64_t;

struct server_status {
    static DATA_BASE_EXECUTION_STATUS data_base_status;
    static JWT_EXECUTION_STATUS JWT_status;
    static FILE_PARSER_EXECUTION_STATUS file_parse_status;

    static auto get_status() -> int;
};