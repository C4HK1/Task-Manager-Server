#include "profile.h"

nlohmann::json profile::to_json() const {
    nlohmann::json rooms;

    for (auto room : this->rooms) {
        rooms.push_back(room.to_json());
    }

    return {
            {"ID", this->ID},
            {"name", this->name},
            {"rooms", rooms}
        };
}