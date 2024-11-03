#include "server/server_status.h"

#include "services/data_base_manager.h"
#include "services/JWT_manager.h"
#include "services/file_parser.h"

DATA_BASE_EXECUTION_STATUS server_status::data_base_status{DATA_BASE_COMPLETED_SUCCESSFULY};
JWT_EXECUTION_STATUS server_status::JWT_status{JWT_COMPLETED_SUCCESSFULY};
FILE_PARSER_EXECUTION_STATUS server_status::file_parse_status{FILE_PARSER_COMPLETED_SUCCESSFULY};

auto server_status::get_status() -> int {
    return data_base_status | (JWT_status << DATA_BASE_SHIFT) | (file_parse_status << (DATA_BASE_SHIFT + JWT_SHIFT));
}