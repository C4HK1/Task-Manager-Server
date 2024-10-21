#pragma once

#include <sys/types.h>
#include <vector>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include "../Room/room.h"

struct profile {
    u_int64_t ID;
    std::string name;
    std::string login;
    std::string password;

    nlohmann::json to_json() const;
};