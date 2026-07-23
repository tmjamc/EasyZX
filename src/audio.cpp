#include <cstdint>
#include <vector>
#include <mutex>
#include <format>

#include "RtAudio.h"
#include "audio.h"
#include "main.h"
#include "settings.h"
#include "win_app.h"

namespace audio
{
    namespace
    {
        constexpr unsigned int OUTPUT_HZ = 44100;
        constexpr unsigned int SAMPLES_PER_FRAME = OUTPUT_HZ / 50;

        double period = 0.0;
        double periodDelta = 0.0;
        double periodCount = 0.0;
        std::vector<RtAudio::DeviceInfo> devices;

        RtAudio rtAudio;
        std::mutex mutex;
        std::vector<int16_t> audioBuffer;

        int rtAudioCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData)
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
            if (true /*!_zx->paused*/)
            {
                // This block implements a simple feedback-control loop that nudges the
                // emulated CPU "period" (cycles per audio sample) up or down each frame
                // to keep the audio buffer size close to what the callback expects.
                // It's a form of clock-drift / sample-rate compensation so the emulator's
                // internal timing stays in sync with the audio thread instead of drifting.

                if (audioBuffer.size() > dBufferFrames)
                {
                    // Buffer has MORE samples than needed: audio is being produced
                    // faster than it's being consumed, so we need to slow production
                    // down by lengthening the period (more emulated cycles per sample).

                    // If periodDelta was previously being adjusted in the opposite
                    // direction (negative, i.e. shortening the period), reset it to 0
                    // before starting to adjust in this new direction. This avoids
                    // fighting a previous correction that's no longer needed.
                    if (periodDelta < 0.0)
                    {
                        periodDelta = 0.0;
                    }
                    // Nudge periodDelta upward by a fixed step to lengthen the period.
                    periodDelta += 0.01;
                }
                else if (audioBuffer.size() < dBufferFrames)
                {
                    // Buffer has FEWER samples than needed: audio is being consumed
                    // faster than it's being produced, so we need to speed production
                    // up by shortening the period (fewer emulated cycles per sample).

                    // If periodDelta was previously being adjusted in the opposite
                    // direction (positive, i.e. lengthening the period), reset it to 0
                    // before starting to adjust in this new direction.
                    if (periodDelta > 0.0)
                    {
                        periodDelta = 0.0;
                    }
                    // Nudge periodDelta downward by a fixed step to shorten the period.
                    periodDelta -= 0.01;
                }
                else
                {
                    // Buffer size exactly matches the target: no correction needed.
                    periodDelta = 0.0;
                }
            }

            // Remove remaining samples if there are too many
            if (audioBuffer.size() > dBufferFrames + dBufferFrames / 10)
            {
                audioBuffer.erase(audioBuffer.begin(), audioBuffer.end() - dBufferFrames);
                periodDelta = 0.0;
            }

            mutex.unlock();

            // Compute the actual period used for emulation timing this frame:
            // base period (cycles per frame divided by samples per frame) plus
            // the accumulated correction (periodDelta) from the logic above.
            period = (double)main::currentModel->tactsPerFrame / (double)SAMPLES_PER_FRAME + periodDelta;

            return 0;
        }
    }

    void reset()
    {
        cleanUp();
        
        win_app::info(std::format("AUDIO -> API Name: {}", rtAudio.getApiDisplayName(rtAudio.getCurrentApi())).c_str());

        period = (double)main::currentModel->tactsPerFrame / (double)SAMPLES_PER_FRAME;

        std::vector<unsigned int> deviceIds = rtAudio.getDeviceIds();
        if (deviceIds.size() < 1)
        {
            win_app::error("AUDIO -> No audio devices found!");
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
            win_app::info("AUDIO -> Selecting default device");
        }

        parameters.nChannels = 2;
        parameters.firstChannel = 0;
        unsigned int sampleRate = OUTPUT_HZ;
        unsigned int bufferFrames = SAMPLES_PER_FRAME;

        if (rtAudio.openStream(&parameters,
                               NULL,
                               RTAUDIO_SINT16,
                               sampleRate,
                               &bufferFrames,
                               rtAudioCallback,
                               NULL))
        {
            win_app::error(rtAudio.getErrorText().c_str());
            return;
        }

        if (rtAudio.startStream())
        {
            win_app::error(rtAudio.getErrorText().c_str());
            return;
        }
    }

    void cleanUp()
    {
        if (rtAudio.isStreamRunning())
        {
            rtAudio.stopStream();
            win_app::info("AUDIO -> stopStream()");
        }
        
        if (rtAudio.isStreamOpen())
        {
            rtAudio.closeStream();
            win_app::info("AUDIO -> closeStream()");
        }
    }

    void addSample(int16_t sample)
    {
        mutex.lock();
        audioBuffer.push_back(sample);
        mutex.unlock();
    }

    bool tact()
    {
        periodCount -= 1.0;

        if (periodCount <= 0.0)
        {
            periodCount += period;
            return true;
        }

        return false;
    }
}