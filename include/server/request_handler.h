#pragma once

class JWT_manager;
class data_base_manager;

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>

class request_handler {   
    http::request<http::dynamic_body> *request;
    http::response<http::dynamic_body> *response;

    json_t request_data;

    JWT_manager *jwt;
    data_base_manager *data_base;
public:
    request_handler(http::request<http::dynamic_body> *request, http::response<http::dynamic_body> *response);
    ~request_handler();

    auto get_request_handler() -> void;
    auto post_request_handler() -> void;
    auto delete_request_handler() -> void;
    auto patch_request_handler() -> void;
 };