#pragma once

#include <glib.h>
#include <librdkafka/rdkafka.h>

namespace services {
    namespace kafka {
        class producer {
            rd_kafka_t *prod;
            rd_kafka_conf_t *conf;
            int message_counter;
            char *topic;
        public:
            producer();
            ~producer();

            void set_config(rd_kafka_conf_t *conf, 
                            const char *key, 
                            const char *value);
            static void dr_msg_cb (rd_kafka_t *kafka_handle,
                                const rd_kafka_message_t *rkmessage,
                                void *opaque);
            int send_message(char *key, char *value);
        };
    }
}