#pragma once

#include <boost/mysql/row.hpp>
#include <boost/mysql/datetime.hpp>
#include <string>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <sys/types.h>

struct task {
    u_int64_t room_creator_ID = 0;
    std::string room_name;
    u_int64_t creator_ID = 0;
    std::string name;

    std::string description;

    std::string label;
    u_int64_t status = 0;

    boost::mysql::datetime creation_time{};
    boost::mysql::datetime deadline{};

    //Additional information
    std::string creator_name;

    task() = default;
    task(std::vector<boost::mysql::field> task);

    auto to_json() const -> nlohmann::json;
};