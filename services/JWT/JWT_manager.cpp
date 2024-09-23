#include <cstdint>
#include <jwt/jwt.hpp>
#include <string>
#include <time.h>
#include <iostream>

#include "JWT_manager.h"
#include "../Text File Parser/text_file_parser.h"

JWT_manager::JWT_manager(const std::string &public_key_path, const std::string &private_key_path)
{
    public_key_ = text_file_parser::read_file_to_string(private_key_path);
    private_key_ = text_file_parser::read_file_to_string(private_key_path);
}

auto JWT_manager::validate_jwt_token(http::request<http::dynamic_body> request) -> jwt::jwt_object
{
    std::string authorizationHeader;
    jwt::jwt_object result;

    for (auto &header : request.base())
    {
        auto headerValue = std::string(header.value());
        if (header.name() == http::field::authorization)
        {
            using namespace jwt::params;
            try {
                result = jwt::decode(headerValue, algorithms({"RS256"}), verify(false), secret(public_key_));
            } catch(std::exception exception) {
                return result;
            }
        }
    }

    return result;
}

auto JWT_manager::create_jwt(std::string &login, std::string &password, uint64_t time_to_live) -> std::string
{
    time_t current_time = time(NULL);

    jwt::jwt_object obj{jwt::params::algorithm("RS256"), jwt::params::secret(private_key_), jwt::params::payload({{"login", login}, {"password", password}, {"destroy_time", std::to_string(current_time + time_to_live)}})};

    std::error_code ec{};
    auto sign = obj.signature(ec);

    if (ec) {
        return "";
    }

    return sign;
}