#include <mutex>
#include <chrono>

#include "main.h"
#include "win_app.h"
#include "display.h"
#include "memory.h"
#include "ula.h"
#include "z80.h"
#include "settings.h"
#include "tape.h"
#include "beta_disk.h"
#include "wd_1793.h"
#include "audio.h"
#include "beeper.h"
#include "ay_3_8912.h"

namespace main
{
    namespace
    {
        constexpr int FRAME_MICROSECONDS = 20 * 1000;
        
        std::thread emulationThread;
        HANDLE frameTimer;
        LARGE_INTEGER frameDueTime{};
        std::chrono::steady_clock::time_point currentFrameTime;

        void reset()
        {
            for (const Model &model : models)
            {
                if (model.id == settings::current.modelId)
                {
                    currentModel = &model;
                    break;
                }
            }

            audio::reset();
            z80::reset();
            memory::reset();
            ula::reset();
            beeper::reset();
            tape::reset();
            beta_disk::reset();

            
        // // // TODO: take beta disk coinfig from settings
        // // wd_1793::insertDisk(0, "C:\\Users\\jam\\Documents\\Projects\\EasyZX_Deploy\\demos\\pentagon\\across_the_edge_by_demarche_fix_0.trd");
        // wd_1793::insertDisk(0, "C:\\Users\\jam\\Documents\\Projects\\EasyZX_Deploy\\demos\\pentagon\\InColor(Pentagon).trd");
        // // wd_1793::insertDisk(0, "C:\\Users\\jam\\Documents\\Projects\\EasyZX_Deploy\\demos\\pentagon\\OldSkoolCodingOldSchoolStyle.trd");
        // // wd_1793::insertDisk(0, "C:\\Users\\jam\\Documents\\Projects\\EasyZX_Deploy\\demos\\pentagon\\summer.trd");
        // // wd_1793::insertDisk(0, "C:\\Users\\jam\\Documents\\Projects\\EasyZX_Deploy\\demos\\pentagon\\esprit.trd");

        // ay_3_8912::init();

        tape::load("C:\\Users\\jam\\Documents\\Projects\\tapes\\Skool Daze.tzx");

        }

        void run()
        {
            // Create Waitable Timer for better accuracy
            frameTimer = CreateWaitableTimerEx(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
            
            emulationThreadReady = true;

            bool interruptRequested = false;
            while (win_app::running)
            {
                interruptRequested = false;
                if (currentTact < currentModel->interruptSignalTacts)
                {
                    interruptRequested = z80::requestInterrupt();
                }

                if (!interruptRequested)
                {
                    if (beta_disk::enabled)
                    {
                        if (beta_disk::romEnabled)
                        {
                            if (z80::registers.pc.w >= 0x4000)
                            {
                                memory::banks[0] = memory::romPages[memory::activeRomPage];
                                beta_disk::romEnabled = false;
                            }
                        }
                        else
                        {
                            if ((currentModel->romPagesCount == 1 || memory::activeRomPage == 1) && z80::registers.pc.b.h == 0x3d)
                            {
                                memory::banks[0] = beta_disk::rom;
                                beta_disk::romEnabled = true;
                            }
                        }
                    }

                    if (!tape::playing && z80::registers.pc.w == 0x056c)
                    {
                        tape::play();
                    }

                    // if (!tape::Started() && z80::registers.pc.w == 0x056c)
                    // {
                    //     tape::StartTape();
                    // }

                    z80::executeInstruction();
                }
            }

            emulationThreadReady = false;

            CloseHandle(frameTimer);
        }

        void startEmulationThread()
        {
            emulationThread = std::thread(run);
        }

        void stopEmulationThread()
        {
            if (emulationThread.joinable())
            {
                emulationThread.join();
            }
        }

        void waitForNextFrame()
        {
            // Do not throttle the frame rate
            if (keyStates[VK_F9])
            {
                const uint64_t frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - currentFrameTime).count();
                if (frameTime < 20)
                {
                    return;
                }
            }

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

    bool emulationThreadReady = false;
    const Model* currentModel;
    int currentTact = 0;
    int currentFrame = 0;
	bool* keyStates;

    void start()
    {
        reset();
        
        keyStates = new bool[0x100]{};
        display::startRenderThread();
        startEmulationThread();
    }

    void stop()
    {
        stopEmulationThread();
        display::stopRenderThread();
        delete[] keyStates;

        audio::cleanUp();
        z80::cleanUp();
        memory::cleanUp();
        ula::cleanUp();
        // beeper::cleanUp();
        tape::cleanUp();
        beta_disk::cleanUp();
        // ay_3_8912::cleanUp();
    }

    void tact()
    {
        // if (beta_disk::enabled)
        // {
        //     wd_1793::tact();
        // }

        tape::tact();

        beeper::tact();

        if (ay_3_8912::enabled)
        {
            ay_3_8912::tact();
        }

        if (audio::tact())
        {
            int16_t sample = tape::filter.getSample() - tape::MAX_AMPLITUDE / 2;

            sample += beeper::filter.getSample();

            if (ay_3_8912::enabled)
            {
                sample += ay_3_8912::filterA.getSample() - ay_3_8912::MAX_AMPLITUDE / 2;
                sample += ay_3_8912::filterB.getSample() - ay_3_8912::MAX_AMPLITUDE / 2;
                sample += ay_3_8912::filterC.getSample() - ay_3_8912::MAX_AMPLITUDE / 2;
            }

            // Twice for left and right channels
            audio::addSample(sample);
            audio::addSample(sample);
        }

        if (++currentTact == currentModel->tactsPerFrame)
        {
            ula::updateDisplayBuffer();

            currentTact = 0;
            waitForNextFrame();
            ++currentFrame;
        }
    }
}