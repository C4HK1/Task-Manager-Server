#pragma once

#include <vector>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include "../Task/task.h"

struct room {
    std::string ID;
    std::string creator_id;
    std::string creator_name;
    std::string label;

    std::vector<task> tasks;

    nlohmann::json to_json() const;
};