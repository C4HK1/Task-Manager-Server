#include <boost/format/format_fwd.hpp>
#include <ctime>
#include <nlohmann/json_fwd.hpp>
#include <chrono>

#include "task.h"

auto task::to_json() const -> nlohmann::json {
    return {
            {"room_creator_ID", this->room_creator_ID},
            {"room_name", this->room_name},
            {"creator_ID", this->creator_ID},
            {"name", this->name},
            {"label", this->label},
            {"status", this->status},
            {"creation_time", this->creation_time.valid() ? std::chrono::system_clock::to_time_t(this->creation_time.as_time_point()) : std::chrono::system_clock::to_time_t({})},
            {"deadline", this->deadline.valid() ? std::chrono::system_clock::to_time_t(this->deadline.as_time_point()) : std::chrono::system_clock::to_time_t({})},
        };
}