#include <bits/types/timer_t.h>
#include <chrono>

#include "models/timer.h"

timer::timer(time_t time_to_live) {
    auto creation_time = std::chrono::system_clock::now();
    this->creation_time = this->convert_time_point_to_datetime(creation_time);
    this->deadline = this->convert_time_point_to_datetime(creation_time + std::chrono::minutes(time_to_live));
}

boost::mysql::datetime timer::convert_time_point_to_datetime(time_point time) {
    auto time_t = std::chrono::system_clock::to_time_t(time);
    auto time_tm = std::localtime(&time_t);

    boost::mysql::datetime datetime = {
        static_cast<uint16_t>(time_tm->tm_year + YEAR_SHIFT),
        static_cast<uint8_t>(time_tm->tm_mon + MONTH_SHIFT),
        static_cast<uint8_t>(time_tm->tm_mday),
        static_cast<uint8_t>(time_tm->tm_hour),
        static_cast<uint8_t>(time_tm->tm_min),
        static_cast<uint8_t>(time_tm->tm_sec),
        static_cast<uint32_t>(0)};

    return datetime;
}

time_t timer::convert_datetime_to_time_t(boost::mysql::datetime datetime) {
    return datetime.valid() ? std::chrono::system_clock::to_time_t(datetime.as_time_point()) : std::chrono::system_clock::to_time_t({});
}
