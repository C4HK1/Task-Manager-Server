#pragma once

#include <boost/mysql/row.hpp>
#include <sys/types.h>
#include <vector>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

namespace models {
    struct invite {
        u_int64_t sender_ID = 0;
        u_int64_t receiver_ID = 0;
        u_int64_t room_creator_ID = 0;
        std::string room_name;

        //Additional info
        std::string sender_name;
        std::string receiver_name;

        invite() = default;
        invite(std::vector<boost::mysql::field> invite);

        auto to_json() const -> nlohmann::json;
    };
}