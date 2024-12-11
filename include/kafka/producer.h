#pragma once

#include <librdkafka/rdkafkacpp.h>
#include <iostream>

namespace kafka {
    constexpr const char *brokers{"localhost:9092"};

    class producer {
    public:
        static void send_message(const std::string &topic_name, const std::string &key, const std::string &value);
    };
}