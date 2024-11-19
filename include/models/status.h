#pragma once

#include <sys/types.h>


namespace services {
    enum DATA_BASE_EXECUTION_STATUS : u_int64_t;
    enum JWT_EXECUTION_STATUS : u_int64_t;
    enum FILE_PARSER_EXECUTION_STATUS : u_int64_t;
}

namespace server {
    enum REQUEST_STATUS : u_int64_t;
}

namespace models {
    struct status {
        static services::DATA_BASE_EXECUTION_STATUS data_base_status;
        static services::JWT_EXECUTION_STATUS JWT_status;
        static services::FILE_PARSER_EXECUTION_STATUS file_parse_status;
        static server::REQUEST_STATUS request_status;

        static auto get_status() -> int;
    };
}