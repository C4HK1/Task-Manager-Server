#include "global_status.h"

DATA_BASE_EXECUTION_STATUS global_status::data_base_status{DATA_BASE_COMPLETED_SUCCESSFULY};
JWT_EXECUTION_STATUS global_status::JWT_status{JWT_COMPLETED_SUCCESSFULY};
FILE_PARSER_EXECUTION_STATUS global_status::file_parse_status{FILE_PARSER_COMPLETED_SUCCESSFULY};

auto global_status::get_status() -> int {
    return data_base_status | (JWT_status << DATA_BASE_SHIFT) | (file_parse_status << (DATA_BASE_SHIFT + JWT_SHIFT));
}