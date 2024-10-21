#pragma once

#include <boost/beast/http.hpp>
#include <cstddef>
#include <jwt/jwt.hpp>
#include <string>
#include <sys/types.h>
#include <bitset>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>

enum JWT_EXECUTION_STATUS {
    JWT_COMPLETED_SUCCESSFULY,
    JWT_DECODING_FILED,
    JWT_NO_TOKEN_HEADER,
    JWT_TIME_LIMIT_EXECEEDED,
    JWT_CREATION_FAILED,
};

constexpr size_t JWT_SHIFT = 3;

constexpr char DEFAULT_PUB_KEY_POSITION[] = "../jwtRS256.key.pub";
constexpr char DEFAULT_KEY_POSITION[] = "../jwtRS256.key";

class JWT_manager
{
    std::string public_key_;
    std::string private_key_;

public:
    JWT_manager(const std::string  &public_key_path = DEFAULT_PUB_KEY_POSITION, const std::string  &private_key_path = DEFAULT_KEY_POSITION);

    auto validate_jwt_token(http::request<http::dynamic_body> request, json_t &result_data) -> JWT_EXECUTION_STATUS;
    auto create_jwt(const std::string &login, const std::string &password, u_int64_t time_to_live, std::string &result_jwt) -> JWT_EXECUTION_STATUS;
};
