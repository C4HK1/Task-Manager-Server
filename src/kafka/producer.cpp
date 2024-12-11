#include "kafka/producer.h"

#include <librdkafka/rdkafkacpp.h>
#include <iostream>
#include <string>

void kafka::producer::send_message(const std::string &topic_name, const std::string &key, const std::string &value) {
    std::string errstr;

    RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    conf->set("bootstrap.servers", brokers, errstr);

    RdKafka::Producer *producer = RdKafka::Producer::create(conf, errstr);
    if (!producer) {
        std::cerr << "Failed to create producer: " << errstr << std::endl;
        delete conf;
        return;
    }

    RdKafka::Topic *topic = RdKafka::Topic::create(producer, topic_name, nullptr, errstr);
    if (!topic) {
        std::cerr << "Failed to create topic: " << errstr << std::endl;
        delete producer;
        delete conf;
        return;
    }

    // Отправка сообщения
    RdKafka::ErrorCode err = producer->produce(
        topic, RdKafka::Topic::OFFSET_END, RdKafka::Producer::RK_MSG_COPY,
        const_cast<char *>(value.c_str()), value.size(), &key, nullptr);

    if (err != RdKafka::ERR_NO_ERROR) {
        std::cerr << "Error while producing message: " << RdKafka::err2str(err) << std::endl;
    }

    while (producer->flush(1000));

    std::cout << "message: " << key << " " << value << " sended on topic: " << topic << std::endl; 

    delete topic;
    delete producer;
    delete conf;
}