#include <nlohmann/json_fwd.hpp>

#include "task.h"

nlohmann::json task::to_json() const
{
    return {
            {"ID", this->ID},
            {"room_id", this->room_id},
            {"creator_id", this->creator_id},
            {"creator_name", this->creator_name},
            {"label", this->label},
        };
}