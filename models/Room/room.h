#pragma once

#include <sys/types.h>
#include <vector>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include "../Task/task.h"

struct room {
    u_int64_t ID;
    u_int64_t creator_id;
    std::string label;

    std::string creator_name;

    nlohmann::json to_json() const;
};