#pragma once

#include <stdint.h>

const uint32_t k_max_message_size = 4096;

enum {
    RES_OK = 0,
    RES_ERR = 1,
    RES_NX = 2,
};
