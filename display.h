#pragma once

#include <cstdint>
#include <thread>

namespace display
{
	void startThread();
	void stopThread();
	void setViewport(int width, int height);
}