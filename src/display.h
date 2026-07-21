#pragma once

#include <mutex>
#include <condition_variable>

namespace display
{
	constexpr int GL_DISPLAY_BUFFER_WIDTH = 352;
	constexpr int GL_DISPLAY_BUFFER_HEIGHT = 288;
	constexpr int GL_DISPLAY_BUFFER_SIZE = GL_DISPLAY_BUFFER_WIDTH * GL_DISPLAY_BUFFER_HEIGHT;
	constexpr int GL_DISPLAY_BUFFER_SIZE_BYTES = GL_DISPLAY_BUFFER_SIZE << 2;
	constexpr float DISPLAY_BUFFER_WIDTH = GL_DISPLAY_BUFFER_WIDTH;
	constexpr float DISPLAY_BUFFER_HEIGHT = GL_DISPLAY_BUFFER_HEIGHT;
	constexpr int GL_MAX_BORDER_SIZE = 48;
	constexpr float MAX_BORDER_SIZE = GL_MAX_BORDER_SIZE;

	extern uint32_t* displayBuffer;
	extern bool frameReady;
	extern std::mutex frameReadyMutex;
	extern std::condition_variable frameReadyConditionVariable;

	void startRenderThread();
	void stopRenderThread();
	void setViewport(int width, int height);
}