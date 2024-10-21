#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

struct task {
    u_int64_t ID;
    u_int64_t room_id;
    std::string label;
    u_int64_t creator_id;

    std::string creator_name;

    nlohmann::json to_json() const;
};