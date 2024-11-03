#pragma once

#include <boost/mysql/row.hpp>
#include <sys/types.h>
#include <vector>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

struct room {
    u_int64_t creator_ID = 0;
    std::string name;

    std::string description;

    //Additional information
    std::string creator_name;

    room() = default;
    room(std::vector<boost::mysql::field> room);

    auto to_json() const -> nlohmann::json;
};