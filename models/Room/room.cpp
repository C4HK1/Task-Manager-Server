#include "room.h"
#include <nlohmann/json_fwd.hpp>
#include <iostream>

nlohmann::json room::to_json() const {
    nlohmann::json tasks;

    for (auto task : this->tasks) {
        tasks.push_back(task.to_short_json());
    }

    return {
            {"creator_id", this->creator_id},
            {"creator_name", this->creator_name},
            {"label", this->label},
            {"tasks", tasks},
        };
}