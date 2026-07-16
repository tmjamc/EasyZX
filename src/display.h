#pragma once

#include <mutex>
#include <condition_variable>

namespace display
{
	extern bool frameReady;
	extern std::mutex frameReadyMutex;
	extern std::condition_variable frameReadyConditionVariable;

	void startRenderThread();
	void stopRenderThread();
	void setViewport(int width, int height);
}