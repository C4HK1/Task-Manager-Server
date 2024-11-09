#pragma once

#include "jwt/jwt.hpp"
#include <boost/asio/ip/basic_resolver.hpp>
#include <boost/exception/exception.hpp>
#include <boost/mysql/datetime.hpp>
#include <boost/mysql/error_code.hpp>
#include <boost/mysql/error_with_diagnostics.hpp>
#include <boost/mysql/time.hpp>
#include <boost/url.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <boost/mysql/field_view.hpp>
#include <boost/mysql/rows_view.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <boost/format.hpp>
#include <boost/mysql/tcp_ssl.hpp>
#include <stdlib.h>
#include <sys/types.h>
#include <utility>
#include <vector>

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