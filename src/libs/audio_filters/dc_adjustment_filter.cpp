#include "dc_adjustment_filter.h"

DcAdjustmentFilter::DcAdjustmentFilter(int maxDcBufferLength)
{
    _maxDcBufferLength = maxDcBufferLength;
}

DcAdjustmentFilter::~DcAdjustmentFilter()
{
    cleanUp();
}

void DcAdjustmentFilter::setBufferPercentLength(int value)
{
    cleanUp();

    _bufferLength = (_maxDcBufferLength * value) / 100;
    if (!_bufferLength)
    {
        _bufferLength = 1;
    }
    _buffer = new int[_bufferLength] {};
    
    _position = 0;
    _sum = 0;
}

void DcAdjustmentFilter::add(int16_t value)
{
    _sum -= _buffer[_position];
    _sum += value;
    _buffer[_position] = value;

    if (++_position == _bufferLength)
    {
        _position = 0;
    }
}

int16_t DcAdjustmentFilter::getSample()
{
    return _sum / _bufferLength;
}

void DcAdjustmentFilter::cleanUp()
{
    if (_buffer != nullptr)
    {
        delete[_bufferLength] _buffer;
        _buffer = nullptr;
    }
}