#include "models/status.h"

#include "services/data_base_manager.h"
#include "services/JWT_manager.h"
#include "services/file_parser.h"
#include "server/request_handler.h"

services::DATA_BASE_EXECUTION_STATUS models::status::data_base_status{services::DATA_BASE_COMPLETED_SUCCESSFULY};
services::JWT_EXECUTION_STATUS models::status::JWT_status{services::JWT_COMPLETED_SUCCESSFULY};
services::FILE_PARSER_EXECUTION_STATUS models::status::file_parse_status{services::FILE_PARSER_COMPLETED_SUCCESSFULY};
server::REQUEST_STATUS models::status::request_status{server::REQUEST_COMPLETED_SUCCESSFULY};

auto models::status::get_status() -> int {
    return data_base_status | (JWT_status << services::DATA_BASE_SHIFT) | (file_parse_status << (services::DATA_BASE_SHIFT + services::JWT_SHIFT)) | (request_status << (services::DATA_BASE_SHIFT + services::JWT_SHIFT + services::FILE_PARSER_SHIFT));
}