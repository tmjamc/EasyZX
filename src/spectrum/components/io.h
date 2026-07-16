#pragma once

#include <cstdint>

namespace io
{
    uint8_t read(uint16_t port);
	void write(uint16_t port, uint8_t data);
}