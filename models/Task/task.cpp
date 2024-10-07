#include "task.h"
#include <nlohmann/json_fwd.hpp>

nlohmann::json task::to_json() const
{
    return {
            {"creator_id", this->creator_id},
            {"creator_name", this->creator_name},
            {"label", this->label},
        };
}
