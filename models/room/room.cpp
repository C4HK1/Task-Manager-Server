#include "room.h"

auto room::to_json() const -> nlohmann::json {
    return {
            {"creator ID", this->creator_ID},
            {"name", this->name},
            {"description", this->description},

            {"creator name", this->creator_name},
        };
}