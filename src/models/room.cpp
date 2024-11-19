#include "models/room.h"

models::room::room(std::vector<boost::mysql::field> room) {
    this->creator_ID = room.at(0).get_uint64();
    this->name = room.at(1).get_string();
    this->description = room.at(2).get_string();
}

auto models::room::to_json() const -> nlohmann::json {
    return {
            {"creator ID", this->creator_ID},
            {"name", this->name},
            {"description", this->description},

            {"creator name", this->creator_name},
        };
}