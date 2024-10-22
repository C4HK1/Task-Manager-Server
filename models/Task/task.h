#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

struct task {
    u_int64_t ID = 0;
    u_int64_t room_id = 0;
    std::string label;
    u_int64_t creator_id = 0;

    std::string creator_name;

    nlohmann::json to_json() const;
};