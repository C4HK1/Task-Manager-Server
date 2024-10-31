#include "profile.h"

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