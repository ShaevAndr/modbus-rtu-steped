#pragma once
#include <cstdint>
#include <cstddef>

namespace ModbusCRC {
    uint16_t calculate(const unsigned char *data, size_t length);
}
