#pragma once

#include "../../services/JWT/JWT_manager.h"
#include "../../services/Data Base/data_base_manager.h"

struct gloabl_status {
    JWT_EXECUTION_STATUS JWT_status;
    DATA_BASE_EXECUTION_STATUS data_base_status;

    auto get_status() -> int;
};