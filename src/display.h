#pragma once

#include <condition_variable>
#include <cstdint>
#include <mutex>

namespace display
{
    constexpr int GL_DISPLAY_BUFFER_WIDTH = 352;
    constexpr int GL_DISPLAY_BUFFER_HEIGHT = 288;
    constexpr int GL_DISPLAY_BUFFER_SIZE = GL_DISPLAY_BUFFER_WIDTH * GL_DISPLAY_BUFFER_HEIGHT;
    constexpr int GL_DISPLAY_BUFFER_SIZE_BYTES = GL_DISPLAY_BUFFER_SIZE * static_cast<int>(sizeof(uint32_t));

    constexpr float DISPLAY_BUFFER_WIDTH = static_cast<float>(GL_DISPLAY_BUFFER_WIDTH);
    constexpr float DISPLAY_BUFFER_HEIGHT = static_cast<float>(GL_DISPLAY_BUFFER_HEIGHT);

    constexpr int GL_MAX_BORDER_SIZE = 48;
    constexpr float MAX_BORDER_SIZE = static_cast<float>(GL_MAX_BORDER_SIZE);

    extern uint32_t* displayBuffer;
    extern bool frameReady;
    extern std::mutex frameReadyMutex;
    extern std::condition_variable frameReadyConditionVariable;

    void startRenderThread();
    void stopRenderThread();
    void setViewport(int width, int height);
}
