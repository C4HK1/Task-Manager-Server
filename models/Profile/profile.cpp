#include "profile.h"

nlohmann::json profile::to_json() const {
    return {
            {"ID", this->ID},
            {"name", this->name}
        };
}