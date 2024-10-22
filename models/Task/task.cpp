#include <nlohmann/json_fwd.hpp>

#include "task.h"

nlohmann::json task::to_json() const
{
    return {
            {"room_id", this->room_id},
            {"creator_id", this->creator_id},
            {"creator_name", this->creator_name},
            {"label", this->label}
        };
}

nlohmann::json task::to_short_json() const
{
    return {
            {"room_id", this->room_id},
            {"creator_name", this->creator_name},
            {"label", this->label}
        };
}