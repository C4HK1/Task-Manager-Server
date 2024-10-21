#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

struct task {
    std::string ID;
    std::string room_id;
    std::string label;
    std::string creator_id;
    std::string creator_name;

    nlohmann::json to_json() const;
    nlohmann::json to_short_json() const;
};