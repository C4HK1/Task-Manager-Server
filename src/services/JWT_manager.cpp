#include <jwt/jwt.hpp>
#include <string>
#include <sys/types.h>
#include <time.h>

#include "services/JWT_manager.h"
#include "services/file_parser.h"
#include "models/status.h"

extern constexpr u_int64_t TIME_TO_LIVE = 60 * 60 * 24 * 7;

services::JWT_manager::JWT_manager(const std::string &public_key_path, const std::string &private_key_path) {
    auto private_key_reading_status = file_parser::read_text_file_to_string(private_key_path, private_key_);
    auto public_key_reading_status = file_parser::read_text_file_to_string(public_key_path, public_key_);

    models::status::file_parse_status = std::max(private_key_reading_status, public_key_reading_status);
}

auto services::JWT_manager::validate_jwt_token(http::request<http::dynamic_body> request, json_t &result_data) -> JWT_EXECUTION_STATUS {
    for (auto &header : request.base()) {
        auto header_value = std::string(header.value());
        if (header.name() == http::field::authorization) {
            using namespace jwt::params;
            try {
                auto data = jwt::decode(header_value, algorithms({"RS256"}), verify(false), secret(public_key_)).payload().create_json_obj();
                
                if (std::atoi(std::string(data.at("destroy_time")).c_str()) <= time(NULL))
                    return JWT_TIME_LIMIT_EXECEEDED;
                
                result_data = data;

                return JWT_COMPLETED_SUCCESSFULY;
            } catch(std::exception exception) {
                return JWT_DECODING_FILED;
            }
        }
    }

    return JWT_NO_TOKEN_HEADER;
}

auto services::JWT_manager::create_jwt(const u_int64_t ID, const std::string &login, const std::string &password, std::string &result_jwt) -> JWT_EXECUTION_STATUS {
    time_t current_time = time(NULL);

    jwt::jwt_object object{jwt::params::algorithm("RS256"), jwt::params::secret(private_key_), jwt::params::payload({{"ID", std::to_string(ID)}, {"login", login}, {"password", password}, {"destroy_time", std::to_string(current_time + TIME_TO_LIVE)}})};

    std::error_code error_code{};
    auto signature = object.signature(error_code);

    if (error_code)
        return JWT_CREATION_FAILED;

    result_jwt = signature;

    return  JWT_COMPLETED_SUCCESSFULY;
}