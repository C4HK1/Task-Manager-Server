#include "config.h"

config::config() : avatar("avatar"), configuration("config") {
}

auto config::to_json() const -> nlohmann::json {
    return  {
        {"avatar", this->avatar},
        {"configuration", this->configuration},
    };
}
