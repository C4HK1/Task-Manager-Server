#pragma once

#include "../../services/data-base/data_base_manager.h"
#include "../../services/JWT/JWT_manager.h"
#include "../../services/file-parser/file_parser.h"

struct global_status {
    static DATA_BASE_EXECUTION_STATUS data_base_status;
    static JWT_EXECUTION_STATUS JWT_status;
    static FILE_PARSER_EXECUTION_STATUS file_parse_status;

    static auto get_status() -> int;
};