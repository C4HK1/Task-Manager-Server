#pragma once

#include <glib.h>
#include <librdkafka/rdkafka.h>

namespace kafka {
    class producer {
        rd_kafka_t *producer_;
        rd_kafka_conf_t *conf;
    public:
        //Object part
        producer();
        ~producer();

        int send_message(
                const char *topic, 
                const char *key, 
                const char *value);
    };
}