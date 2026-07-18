#include <cstdint>
#include <vector>
#include <mutex>

#include "RtAudio.h"
#include "audio.h"
#include "main.h"
#include "settings.h"

namespace audio
{
    static constexpr unsigned int OUTPUT_HZ = 44100;
	static constexpr unsigned int SAMPLES_PER_FRAME = OUTPUT_HZ / 50;

	int bufferSize;
	double period = 0.0;
	double periodDelta = 0.0;
	std::vector<RtAudio::DeviceInfo> devices;

	RtAudio rtAudio;
	std::mutex mutex;
	std::vector<int16_t> audioBuffer;

    static int rtAudioCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData)
    {
        mutex.lock();

        int16_t *buffer = (int16_t *)outputBuffer;

        // Write interleaved audio data.
        const unsigned int dBufferFrames = nBufferFrames * 2;
        unsigned int samplesWritten = 0;
        while (samplesWritten < dBufferFrames)
        {
            if (samplesWritten >= audioBuffer.size() /*|| _zx->paused*/)
            {
                *buffer++ = 0;
                *buffer++ = 0;
                samplesWritten += 2;
            }
            else
            {
                *buffer++ = audioBuffer[samplesWritten++];
                *buffer++ = audioBuffer[samplesWritten++];
            }
        }

        // Remove samples written to buffer
        if (samplesWritten < audioBuffer.size())
        {
            audioBuffer.erase(audioBuffer.begin(), audioBuffer.begin() + samplesWritten);
        }

        // Adjust period to match samples needed by audio callback
        if (true/*!_zx->paused*/)
        {
            if (audioBuffer.size() > dBufferFrames)
            {
                // if there is more samples than neccesary, increase period to decrease samples for next frame
                if (periodDelta < 0.0)
                {
                    periodDelta = 0.0;
                }
                periodDelta += 0.01;
            }
            else if (audioBuffer.size() < dBufferFrames)
            {
                // if more samples in buffer are needed, decrease period to increase number of samples for next frame
                if (periodDelta > 0.0)
                {
                    periodDelta = 0.0;
                }
                periodDelta -= 0.01;
            }
            else
            {
                periodDelta = 0.0;
            }

            period = (double)main::currentModel->tactsPerFrame / (double)SAMPLES_PER_FRAME + periodDelta;
        }

        // Remove remaining samples if there are too many
        if (audioBuffer.size() > dBufferFrames + dBufferFrames / 10)
        {
            audioBuffer.erase(audioBuffer.begin(), audioBuffer.end() - dBufferFrames);
            periodDelta = 0.0;
        }

        bufferSize = audioBuffer.size();

        mutex.unlock();

        return 0;
    }

    void init()
    {
        period = (double)main::currentModel->tactsPerFrame / (double)SAMPLES_PER_FRAME;

        std::vector<unsigned int> deviceIds = rtAudio.getDeviceIds();
        if (deviceIds.size() < 1)
        {
            std::cout << "\nNo audio devices found!\n";
            exit(0);
        }

        devices.clear();
        for (unsigned int id : deviceIds)
        {
            RtAudio::DeviceInfo info = rtAudio.getDeviceInfo(id);
            if (info.outputChannels == 2)
            {
                devices.push_back(info);
            }
        }

        RtAudio::StreamParameters parameters;

        for (RtAudio::DeviceInfo deviceInfo : devices)
        {
            if (deviceInfo.ID == settings::current.audioDeviceId)
            {
                parameters.deviceId = deviceInfo.ID;
                break;
            }
        }

        if (!parameters.deviceId)
        {
            parameters.deviceId = rtAudio.getDefaultOutputDevice();
            // settings::current.audioDeviceId = parameters.deviceId;
        }

        parameters.nChannels = 2;
        parameters.firstChannel = 0;
        unsigned int sampleRate = OUTPUT_HZ;
        unsigned int bufferFrames = SAMPLES_PER_FRAME;

        if (rtAudio.openStream(
                &parameters,
                NULL,
                RTAUDIO_SINT16,
                sampleRate,
                &bufferFrames,
                rtAudioCallback,
                NULL))
        {
            std::cout << '\n'
                      << rtAudio.getErrorText() << '\n'
                      << std::endl;
            exit(-1); // problem with device settings
        }

        if (rtAudio.startStream())
        {
            std::cout << rtAudio.getErrorText() << std::endl;
            exit(-1); // problem with device settings
        }
    }

    void cleanUp()
    {
        if (rtAudio.isStreamRunning())
        {
            rtAudio.stopStream();
        }

        if (rtAudio.isStreamOpen())
        {
            rtAudio.closeStream();
        }
    }

    void addSample(int16_t sample)
    {
        mutex.lock();
        audioBuffer.push_back(sample);
        mutex.unlock();
    }

    // void clearBuffer()
    // {
    //     mutex.lock();
    //     audioBuffer.clear();
    //     mutex.unlock();
    // }
}