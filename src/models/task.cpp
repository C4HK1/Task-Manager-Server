#include <boost/format/format_fwd.hpp>
#include <ctime>
#include <nlohmann/json_fwd.hpp>

#include "models/timer.h"
#include "models/task.h"

auto task::to_json() const -> nlohmann::json {
    return {
            {"room creator ID", this->room_creator_ID},
            {"room name", this->room_name},
            {"creator ID", this->creator_ID},
            {"name", this->name},
            {"description", this->description},
            {"label", this->label},
            {"status", this->status},
            {"creation time", timer::convert_datetime_to_time_t(this->creation_time)},
            {"deadline", timer::convert_datetime_to_time_t(this->deadline)},

            {"creator name", this->creator_name},
        };
}