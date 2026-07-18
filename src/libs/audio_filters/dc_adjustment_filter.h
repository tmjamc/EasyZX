#pragma once

#include <cstdint>

class DcAdjustmentFilter
{
public:
    DcAdjustmentFilter(int maxDcBufferLength = 882);
    ~DcAdjustmentFilter();
    void setBufferPercentLength(int value);
    void add(int16_t value);
    int16_t getSample();

private:
    int _maxDcBufferLength = 0;
    int _sum = 0;
    int _position = 0;
    int _bufferLength = 0;
    int* _buffer = nullptr;
    void cleanUp();
};