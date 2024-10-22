#pragma once

#include <vector>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include "../Room/room.h"

struct profile {
    std::string ID;
    std::string name;

    nlohmann::json to_json() const;
};