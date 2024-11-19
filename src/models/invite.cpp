#include <boost/mysql/row.hpp>

#include "models/invite.h"

models::invite::invite(std::vector<boost::mysql::field> invite) {
    this->sender_ID = invite.at(0).get_uint64();
    this->receiver_ID = invite.at(1).get_uint64();
    this->room_creator_ID = invite.at(2).get_uint64();
    this->room_name = invite.at(3).get_string();       
}

auto models::invite::to_json() const -> nlohmann::json {
    return {
            {"sender ID", this->sender_ID},
            {"receiver ID", this->receiver_ID},
            {"room creator ID", this->room_creator_ID},
            {"room name", this->room_name},
            {"sender name", this->sender_name},
            {"receiver name", this->receiver_name},
        };
}