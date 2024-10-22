#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

struct task {
    std::string room_id;
    std::string creator_id;
    std::string creator_name;
    std::string label;

    nlohmann::json to_json() const;
    nlohmann::json to_short_json() const;
};