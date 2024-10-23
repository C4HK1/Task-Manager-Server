#include "global_status.h"

auto gloabl_status::get_status() -> int {
    return this->data_base_status | (this->JWT_status << DATA_BASE_SHIFT);
}