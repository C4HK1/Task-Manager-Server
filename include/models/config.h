#pragma once

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <sys/types.h>

constexpr char DEFAULT_CASUAL_AVATAR_POSITION[]{"../default.conf"};
constexpr char DEFAULT_CASUAL_CONFIGURATION_POSITION[]{"../default.png"};

struct config {
    std::string avatar;
    std::string configuration;

    config();
    auto to_json() const -> nlohmann::json;
};