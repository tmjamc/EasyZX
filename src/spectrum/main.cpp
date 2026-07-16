#include <mutex>
#include <chrono>

#include "main.h"
#include "win_app.h"
#include "display.h"
#include "memory.h"
#include "ula.h"
#include "z80.h"

namespace main
{
    bool emulationThreadRunning = false;
	std::thread emulationThread;

    constexpr static int FRAME_MICROSECONDS = 20 * 1000;

	HANDLE frameTimer;
	LARGE_INTEGER frameDueTime{};
	std::chrono::steady_clock::time_point currentFrameTime;

    const Model* currentModel;

    int tack = 0;
    int frame = 0;
	bool* keyStates;

    static void init()
    {
        // Create Waitable Timer for better accuracy
        frameTimer = CreateWaitableTimerEx(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
    }

    static void cleanUp()
    {
        CloseHandle(frameTimer);
    }

    static void run()
    {
        init();
        
        bool interruptRequested = false;
		while (win_app::running)
		{
            interruptRequested = false;
            if (tack < currentModel->interruptSignalTacks)
            {
                interruptRequested = z80::requestInterrupt();
            }

            if (!interruptRequested)
            {
                z80::executeInstruction();
            }
        }

        cleanUp();
    }

    static void startEmulationThread()
    {
        emulationThread = std::thread(run);
        emulationThreadRunning = true;
    }

    static void stopEmulationThread()
    {
        emulationThreadRunning = false;
        if (emulationThread.joinable())
		{
			emulationThread.join();
		}
    }

    void start()
    {        
        // TODO: take model from settings
        currentModel = &SPECTRUM_48K;

        keyStates = new bool[0x100]{};
        memory::init();
        ula::init();
        z80::init();

        display::startRenderThread();
        startEmulationThread();
    }

    void stop()
    {
        stopEmulationThread();
        display::stopRenderThread();

        delete[0x100] keyStates;
        memory::cleanUp();
        ula::cleanUp();
        z80::cleanUp();
    }

    void reset(const Model* model)
    {
        memory::cleanUp();
        ula::cleanUp();

        currentModel = model;

        memory::init();
        ula::init();

        z80::reset();
    }

    void waitForNextFrame()
    {
        // Sync with display thread
        {
            // This code MUST be inside a block to release lock before notify_one() is called,
            // otherwise the waiting thread will be woken up but won't be able to acquire the
            // lock and will go back to sleep
            std::lock_guard<std::mutex> lock(display::frameReadyMutex);
        }
        display::frameReady = true; // Update the variable
        display::frameReadyConditionVariable.notify_one(); // Wake up the waiting thread

        // Wait 20 milliseconds (50 FPS) before starting the next frame
        // This is done in two steps:
        // 1. Wait for 19 milliseconds using Waitable Timer (non-blocking, allows other threads to run)
        // 2. Spin-lock (busy-wait) for the remaining 1 millisecond to achieve better accuracy
        //    (since Waitable Timer can be inaccurate due to OS scheduling)

        // time ellapsed since the start of the current frame
        std::chrono::duration delta = std::chrono::steady_clock::now() - currentFrameTime;

        // time remainig to reach 19 milliseconds of frame time (20 milliseconds, 50 FPS)
        delta = std::chrono::microseconds(19 * 1000) - delta;

        // wait delta time (19 milliseconds since the start of current frame) before starting spin-lock (busy-wait)
        if (delta > std::chrono::microseconds(0))
        {
            frameDueTime.QuadPart = -delta.count() / 100;
            if (!SetWaitableTimer(frameTimer, &frameDueTime, 0, nullptr, nullptr, 0))
            {
                exit(-1);
            }
            if (WaitForSingleObject(frameTimer, INFINITE) != WAIT_OBJECT_0)
            {
                exit(-1);
            }
        }

        // start spin-lock (busy-wait) for the remaining 1 millisecond to match exactly 20 milliseconds of frame time (50 FPS)
        std::chrono::time_point start = currentFrameTime;
        do
        {
            currentFrameTime = std::chrono::steady_clock::now();
        } while (currentFrameTime - start <= std::chrono::microseconds(FRAME_MICROSECONDS));
    }
}