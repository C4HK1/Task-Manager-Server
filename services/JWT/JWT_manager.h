#pragma once

#include <boost/beast/http.hpp>
#include <jwt/jwt.hpp>
#include <string>
#include <sys/types.h>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>


class JWT_manager
{
    std::string public_key_;
    std::string private_key_;

public:
    JWT_manager(const std::string  &public_key_path = "../jwtRS256.key.pub", const std::string  &private_key_path = "../jwtRS256.key");

    auto validate_jwt_token(http::request<http::dynamic_body> request) -> jwt::jwt_object;
    auto create_jwt(const std::string &login, const std::string &password, u_int64_t time_to_live) -> std::string;
};
