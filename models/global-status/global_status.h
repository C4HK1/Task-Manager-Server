#pragma once

#include "../../services/JWT/JWT_manager.h"
#include "../../services/data-base/data_base_manager.h"
#include "../../services/file-parser/file_parser.h"

struct gloabl_status {
    JWT_EXECUTION_STATUS JWT_status;
    DATA_BASE_EXECUTION_STATUS data_base_status;
    FILE_PARSER_EXECUTION_STATUS file_parse_status;

    auto get_status() -> int;
};