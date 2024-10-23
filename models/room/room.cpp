#include "room.h"

auto room::to_json() const -> nlohmann::json {
    return {
            {"creator_ID", this->creator_ID},
            {"name", this->name},
            {"description", this->description},

            {"creator_name", this->creator_name},
        };
}