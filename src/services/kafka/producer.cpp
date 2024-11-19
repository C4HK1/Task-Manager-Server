#include "services/kafka/producer.h"
#include <stdlib.h>
#include <cstring>

services::kafka::producer::producer() {
    char errstr[512];

    // Create client configuration
    conf = rd_kafka_conf_new();
    // User-specific properties that you must set
    set_config(conf, "bootstrap.servers", "<BOOTSTRAP SERVERS>");

    // Fixed properties
    set_config(conf, "security.protocol", "SASL_SSL");

    // Install a delivery-error callback.
    rd_kafka_conf_set_dr_msg_cb(conf, dr_msg_cb);

    // Create the Producer instance.
    prod = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
    if (!prod) {
        g_error("Failed to create new prod: %s", errstr);
        return;
    }

    // Configuration object is now owned, and freed, by the rd_kafka_t instance.
    conf = NULL;

    // Produce data by selecting random values from these lists.
    int message_count = 10;
    topic = "purchases";
}

services::kafka::producer::~producer() {
    // Block until the messages are all sent.
    g_message("Flushing final messages..");
    rd_kafka_flush(prod, 10 * 1000);

    if (rd_kafka_outq_len(prod) > 0) {
        g_error("%d message(s) were not delivered", rd_kafka_outq_len(prod));
    }

    g_message("%d events were produced to topic %s.", message_counter, topic);

    rd_kafka_destroy(prod);
}

void services::kafka::producer::set_config(rd_kafka_conf_t *conf, const char *key, const char *value) {
    char errstr[512];
    rd_kafka_conf_res_t res;

    res = rd_kafka_conf_set(conf, key, value, errstr, sizeof(errstr));
    if (res != RD_KAFKA_CONF_OK) {
        g_error("Unable to set config: %s", errstr);
        conf = nullptr;
    }
}

void services::kafka::producer::dr_msg_cb(rd_kafka_t *kafka_handle,
                                const rd_kafka_message_t *rkmessage,
                                void *opaque) {
    if (rkmessage->err) {
        g_error("Message delivery failed: %s", rd_kafka_err2str(rkmessage->err));
    }             
}

int services::kafka::producer::send_message(char *key, char *value) {
    if (!conf)
        return 1;

    message_counter++;

    size_t key_len = strlen(key);
    size_t value_len = strlen(value);

    rd_kafka_resp_err_t err;

    err = rd_kafka_producev(prod,
                            RD_KAFKA_V_TOPIC(topic),
                            RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
                            RD_KAFKA_V_KEY((void*)key, key_len),
                            RD_KAFKA_V_VALUE((void*)value, value_len),
                            RD_KAFKA_V_OPAQUE(NULL),
                            RD_KAFKA_V_END);

    if (err) {
        g_error("Failed to produce to topic %s: %s", topic, rd_kafka_err2str(err));
        return 1;
    } else {
        g_message("Produced event to topic %s: key = %12s value = %12s", topic, key, value);
    }

    rd_kafka_poll(prod, 0);

    return 0;
}