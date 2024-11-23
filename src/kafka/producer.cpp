#include "kafka/producer.h"
#include "kafka/common.h"
#include <cstdio>

/* Optional per-message delivery callback (triggered by poll() or flush())
 * when a message has been successfully delivered or permanently
 * failed delivery (after retries).
 */

//Object part
kafka::producer::producer() {
    char errstr[512];

    // Create client configuration
    conf = rd_kafka_conf_new();

    // User-specific properties that you must set
    kafka::common::set_config(conf, "bootstrap.servers", KAFKA_BROKERS);

    // Install a delivery-error callback.
    rd_kafka_conf_set_dr_msg_cb(conf, kafka::common::dr_msg_cb);

    // Create the Producer instance.
    producer_ = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
    if (!producer_) {
        g_error("Failed to create new producer: %s", errstr);
        return;
    }

    // Configuration object is now owned, and freed, by the rd_kafka_t instance.
    conf = NULL;

    printf("Producer created\n");
}

kafka::producer::~producer() {
    rd_kafka_destroy(producer_);

    printf("Producer deleted\n");
}


//Methods part
int kafka::producer::create_topic(const char *topic) {
    rd_kafka_resp_err_t err;

    err = rd_kafka_producev(producer_,
                            RD_KAFKA_V_TOPIC(topic),
                            RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
                            RD_KAFKA_V_OPAQUE(NULL),
                            RD_KAFKA_V_END);

    if (err) {
        g_error("Failed to produce to topic %s: %s", topic, rd_kafka_err2str(err));
        return 1;
    } else {
        g_message("Created topic: %s", topic);
    }

    rd_kafka_poll(producer_, 0);

    return 0;
}

int kafka::producer::send_message(
        const char *topic, 
        const char *key, 
        const char *value) {
    size_t key_len = strlen(key);
    size_t value_len = strlen(value);

    rd_kafka_resp_err_t err;

    err = rd_kafka_producev(producer_,
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

    rd_kafka_poll(producer_, 0);

    // Block until the messages are all sent.
    g_message("Flushing message..");

    rd_kafka_flush(producer_, 10 * 1000);

    if (rd_kafka_outq_len(producer_) > 0) {
        g_error("%d message(s) were not delivered", rd_kafka_outq_len(producer_));
    }

    g_message("event were produced to topic.");

    return 0;
}