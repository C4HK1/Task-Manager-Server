#pragma once

#include <boost/beast/core/string_type.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/verb.hpp>
#include <cstddef>
#include <jwt/jwt.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <iostream>
#include <vector>

#include "../../services/JWT/JWT_manager.h"
#include "../../services/data-base/data_base_manager.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>

class request_handler {   
    http::request<http::dynamic_body> &request;
    http::response<http::dynamic_body> &response;

    json_t request_data;

    std::string login;
    std::string password;

    JWT_manager jwt;
    data_base_manager *data_base;

    DATA_BASE_EXECUTION_STATUS data_base_status;
    JWT_EXECUTION_STATUS jwt_status;
public:
    request_handler(http::request<http::dynamic_body> &request, http::response<http::dynamic_body> &response);
    ~request_handler();

    auto get_request_handler() -> void;
    auto post_request_handler() -> void;
    auto delete_request_handler() -> void;
    auto patch_request_handler() -> void;
 };