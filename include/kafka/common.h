#pragma once

#include <glib.h>
#include <librdkafka/rdkafka.h>
#include <cstring>
#include <string>

namespace kafka {
    constexpr char *KAFKA_BROKERS{"localhost"};

    class common {
    public:
        static void set_config(rd_kafka_conf_t *conf, 
                               char *key, 
                               char *value);
        static void dr_msg_cb(rd_kafka_t *kafka_handle,
                              const rd_kafka_message_t *rkmessage,
                              void *opaque);
    };
}