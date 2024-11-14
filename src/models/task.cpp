#include <boost/format/format_fwd.hpp>
#include <boost/mysql/row.hpp>
#include <ctime>
#include <nlohmann/json_fwd.hpp>
#include <iostream>

#include "models/timer.h"
#include "models/task.h"

task::task(std::vector<boost::mysql::field> task) {
    this->room_creator_ID = task.at(0).get_uint64();
    this->room_name = task.at(1).get_string();
    this->creator_ID = task.at(2).get_uint64();
    this->name = task.at(3).get_string();
    this->description = task.at(4).get_string();
    this->label = task.at(5).get_string();
    this->status = task.at(6).get_uint64();
    this->creation_time = task.at(7).get_datetime();
    this->deadline = task.at(8).get_datetime();
}

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