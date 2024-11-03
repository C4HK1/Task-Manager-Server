#include "models/profile.h"
#include <boost/mysql/row.hpp>

profile::profile(std::vector<boost::mysql::field> profile) {
    this->ID = profile.at(0).get_uint64();
    this->name =profile.at(1).get_string();
    this->login = profile.at(2).get_string();
    this->password = profile.at(3).get_string();
    this->email = profile.at(4).get_string();
    this->phone = profile.at(5).get_string();        
}

auto profile::to_json() const -> nlohmann::json {
    u_int64_t ID = 0;
    std::string name;
    std::string login;
    std::string password;

    std::string email;
    std::string phone;

    return {
            {"ID", this->ID},
            {"name", this->name},
            {"login", this->login},
            {"password", this->password},
            {"email", this->email},
            {"phone", this->phone},
        };
}

auto profile::to_public_json() const -> nlohmann::json {
    u_int64_t ID = 0;
    std::string name;
    std::string login;
    std::string password;

    std::string email;
    std::string phone;

    return {
            {"ID", 0},
            {"name", this->name},
            {"login", ""},
            {"password", ""},
            {"email", this->email},
            {"phone", this->phone},
        };
}