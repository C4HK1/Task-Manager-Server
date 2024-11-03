#include "models/config.h"
#include <boost/mysql/row.hpp>
#include <vector>

config::config() : avatar("avatar"), configuration("config") {
}

config::config(std::vector<boost::mysql::field> config) {
    auto avatar = config.at(1).as_blob();
    auto configuration = config.at(2).as_blob();

    this->avatar = std::string(avatar.begin(), avatar.end());
    this->configuration = std::string(configuration.begin(), configuration.end());
}

auto config::to_json() const -> nlohmann::json {
    return  {
        {"avatar", this->avatar},
        {"configuration", this->configuration},
    };
}
