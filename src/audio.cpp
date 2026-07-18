#include <windows.h>
#include <cstdint>
#include <thread>

#include "audio.h"
#include "win_app.h"

namespace audio
{
    #define NUM_BUFFERS 4
    #define BUFFER_SAMPLES 882

    bool audioThreadReady = false;
	std::thread audioThread;
    
    WAVEHDR waveHdr[NUM_BUFFERS];
    int16_t audioData[NUM_BUFFERS][BUFFER_SAMPLES];
    
    HANDLE hAudioEvent = nullptr;
    HWAVEOUT hWaveOut = nullptr;
    
    static void init()
    {
        WAVEFORMATEX wfx = {0};
        wfx.wFormatTag = WAVE_FORMAT_PCM;
        wfx.nChannels = 1;
        wfx.nSamplesPerSec = 44100;
        wfx.wBitsPerSample = 16;
        wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
        wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
        
        HANDLE hAudioEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        MMRESULT result = waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR)hAudioEvent, (DWORD_PTR)nullptr, CALLBACK_EVENT);
        if (result != MMSYSERR_NOERROR)
        {
            win_app::info("AUDIO: ERROR!");
            return;
        }

        for (int i = 0; i < NUM_BUFFERS; i++)
        {
            for (int j = 0; j < BUFFER_SAMPLES; ++j)
            {
                audioData[i][j] = rand() & 0xfff;
            }

            waveHdr[i] = {};
            waveHdr[i].lpData = (LPSTR)audioData[i];
            waveHdr[i].dwBufferLength = BUFFER_SAMPLES * sizeof(int16_t);

            waveOutPrepareHeader(hWaveOut, &waveHdr[i], sizeof(WAVEHDR));
            waveOutWrite(hWaveOut, &waveHdr[i], sizeof(WAVEHDR));
        }
    }

    static void cleanUp()
    {
        waveOutClose(hWaveOut);

        for (int i = 0; i < NUM_BUFFERS; i++)
        {
            waveOutUnprepareHeader(hWaveOut, &waveHdr[i], sizeof(WAVEHDR));
        }
    }

    static void run()
    {
        init();

        audioThreadReady = true;

        while (win_app::running)
        {
            WaitForSingleObject(hAudioEvent, INFINITE);

            for (int i = 0; i < NUM_BUFFERS; i++)
            {
                if (waveHdr[i].dwFlags & WHDR_DONE)
                {
                    for (int j = 0; j < BUFFER_SAMPLES; ++j)
                    {
                        audioData[i][j] = rand() & 0xfff;
                    }

                    waveOutWrite(hWaveOut, &waveHdr[i], sizeof(WAVEHDR));
                    break;
                }
            }
        }

        audioThreadReady = false;

        cleanUp();
    }

    void startAudioThread()
    {
        audioThread = std::thread(run);
    }

    void stopAudioThread()
    {
        if (audioThread.joinable())
		{
			audioThread.join();
		}
    }
}