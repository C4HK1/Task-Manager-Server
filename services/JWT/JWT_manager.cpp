#include <jwt/jwt.hpp>
#include <string>
#include <sys/types.h>
#include <time.h>

#include "JWT_manager.h"
#include "../Text File Parser/text_file_parser.h"

extern constexpr u_int64_t TIME_TO_LIVE = 60 * 60 * 24 * 7;

JWT_manager::JWT_manager(const std::string &public_key_path, const std::string &private_key_path)
{
    public_key_ = text_file_parser::read_file_to_string(private_key_path);
    private_key_ = text_file_parser::read_file_to_string(private_key_path);
}

auto JWT_manager::validate_jwt_token(http::request<http::dynamic_body> request, json_t &result_data) -> JWT_EXECUTION_STATUS
{
    for (auto &header : request.base())
    {
        auto header_value = std::string(header.value());
        if (header.name() == http::field::authorization)
        {
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

auto JWT_manager::create_jwt(const std::string &login, const std::string &password, std::string &result_jwt) -> JWT_EXECUTION_STATUS
{
    time_t current_time = time(NULL);

    jwt::jwt_object obj{jwt::params::algorithm("RS256"), jwt::params::secret(private_key_), jwt::params::payload({{"login", login}, {"password", password}, {"destroy_time", std::to_string(current_time + TIME_TO_LIVE)}})};

    std::error_code ec{};
    auto sign = obj.signature(ec);

    if (ec)
        return JWT_CREATION_FAILED;

    result_jwt = sign;

    return  JWT_COMPLETED_SUCCESSFULY;
}