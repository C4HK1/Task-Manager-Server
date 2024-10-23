#include "room.h"
#include <nlohmann/json_fwd.hpp>

nlohmann::json room::to_json() const {
    return {
            {"creator_ID", this->creator_ID},
            {"name", this->name},
            {"description", this->description},

            {"creator_name", this->creator_name},
        };
}