#pragma once

#include <boost/mysql/date.hpp>
#include <chrono>
#include <ctime>
#include <sys/types.h>
#include <boost/mysql.hpp>

namespace models {
    constexpr u_int64_t YEAR_SHIFT = 1900;
    constexpr u_int64_t MONTH_SHIFT = 1;

    typedef std::chrono::system_clock::time_point time_point;
    typedef std::chrono::system_clock::duration duration;

    struct timer {
        boost::mysql::datetime creation_time;
        boost::mysql::datetime deadline;

        timer();
        timer(time_t time_to_live);

        boost::mysql::datetime get_creation_time();

        static boost::mysql::datetime convert_time_point_to_datetime(time_point time);
        static time_t convert_datetime_to_time_t(boost::mysql::datetime datetime);
    };
}