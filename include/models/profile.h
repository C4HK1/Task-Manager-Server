#pragma once

#include <boost/mysql/row.hpp>
#include <sys/types.h>
#include <vector>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

struct profile {
    u_int64_t ID = 0;
    std::string name;
    std::string login;
    std::string password;

    std::string email;
    std::string phone;

    profile() = default;
    profile(std::vector<boost::mysql::field> profile);

    auto to_json() const -> nlohmann::json;
    auto to_public_json() const -> nlohmann::json;
};