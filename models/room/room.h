#pragma once

#include <sys/types.h>
#include <vector>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include "../task/task.h"

struct room {
    u_int64_t creator_ID = 0;
    std::string name;

    std::string description;

    //Additional information
    std::string creator_name;

    auto to_json() const -> nlohmann::json;
};