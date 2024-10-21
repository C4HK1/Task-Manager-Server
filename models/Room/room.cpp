#include "room.h"
#include <nlohmann/json_fwd.hpp>

nlohmann::json room::to_json() const {
    return {
            {"ID", this->creator_id},
            {"creator_id", this->creator_id},
            {"creator_name", this->creator_name},
            {"label", this->label},
        };
}