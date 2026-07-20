#include "ay_3_8912.h"
#include "main.h"

namespace ay_3_8912
{
    namespace
    {
        constexpr int levels[16] = {
            0x0000, 0x0385, 0x053d, 0x0770,
            0x0ad7, 0x0fd5, 0x15b0, 0x230c,
            0x2b4c, 0x43c1, 0x5a4b, 0x732f,
            0x9204, 0xaff1, 0xd921, 0xffff};

        constexpr int ENV_CONT = 8;
        constexpr int ENV_ATTACK = 4;
        constexpr int ENV_ALT = 2;
        constexpr int ENV_HOLD = 1;

        constexpr int CLOCK_DIVISOR = 16;
        constexpr int CLOCK_RATIO = 2;

        uint8_t selectedRegister = 0;

        int chanASample;
        int chanBSample;
        int chanCSample;

        uint32_t toneLevels[16];
        uint32_t toneTick[3];
        uint32_t toneHigh[3];
        uint32_t noiseTick;
        uint32_t toneCycles;
        uint32_t envCycles;
        uint32_t envInternalTick;
        uint32_t envTick;
        uint32_t tonePeriod[3];
        uint32_t noisePeriod;
        uint32_t envPeriod;
        uint8_t registers[16];

        int32_t envFirst = 1;
        int32_t envRev = 0;
        int32_t envCounter = 15;
        int32_t toneLevel[3];
        int rng = 1;
        int noiseToggle = 0;
        int envshape;
        int level;
        int noiseCount = 0;
        int mixer;
        int toneCount;

        void doTone(int level, unsigned int toneCount, int *sample, int chan)
        {
            *sample = 0;

            toneTick[chan] += toneCount;

            if (toneTick[chan] >= tonePeriod[chan])
            {
                toneTick[chan] -= tonePeriod[chan];
                toneHigh[chan] = !toneHigh[chan];
            }

            if (level)
            {
                if (toneHigh[chan])
                {
                    *sample = level;
                }
                else
                {
                    *sample = 0;
                }
            }
        }
    }

    bool enabled = false;
    DcAdjustmentFilter filterA;
    DcAdjustmentFilter filterB;
    DcAdjustmentFilter filterC;

    void setVolume(int volume)
    {
        for (int i = 0; i < 16; ++i)
        {
            toneLevels[i] = (levels[i] * ((volume * MAX_AMPLITUDE) / 100) /* - 0x8000*/) / 0xffff;
        }
    }

    void reset(int volume, int bufferPercentLength)
    {
        setVolume(volume);
        filterA.setBufferPercentLength(bufferPercentLength);
        filterB.setBufferPercentLength(bufferPercentLength);
        filterC.setBufferPercentLength(bufferPercentLength);

        noiseTick = 0;
        noisePeriod = 0;
        envInternalTick = 0;
        envTick = 0;
        envPeriod = 0;
        toneCycles = 0;
        envCycles = 0;

        for (int i = 0; i < 3; ++i)
        {
            toneTick[i] = 0;
            toneHigh[i] = 0;
            tonePeriod[i] = 1;
        }

        for (int i = 0; i < 16; ++i)
        {
            setRegister(i);
            setRegisterValue(0);
        }

        for (int i = 0; i < 3; ++i)
        {
            toneHigh[i] = 0;
        }

        toneCycles = 0;
        envCycles = 0;
    }

    void init()
    {
        reset(100, 10);

        enabled = true;
    }

    void cleanUp()
    {
    }

    void setRegister(uint8_t reg)
    {
        selectedRegister = reg & 0x0f;
    }

    void setRegisterValue(uint8_t val)
    {
        registers[selectedRegister] = val;

        switch (selectedRegister)
        {

        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        {
            int r = selectedRegister >> 1;
            tonePeriod[r] = (registers[selectedRegister & ~1] | (registers[selectedRegister | 1] & 15) << 8);
            if (!tonePeriod[r])
            {
                tonePeriod[r]++;
            }
            if (toneTick[r] >= tonePeriod[r] * 2)
            {
                toneTick[r] %= tonePeriod[r] * 2;
            }
            break;
        }

        case 6:
            noiseTick = 0;
            noisePeriod = (registers[selectedRegister] & 31);
            break;

        case 11:
        case 12:
            envPeriod = registers[11] | (registers[12] << 8);
            break;

        case 13:
            envInternalTick = envTick = envCycles = 0;
            envFirst = 1;
            envRev = 0;
            envCounter = (registers[13] & ENV_ATTACK) ? 0 : 15;
            break;
        }
    }

    uint8_t getRegisterValue()
    {
        return registers[selectedRegister];
    }

    void tact()
    {
        filterA.add(chanASample);
        filterB.add(chanBSample);
        filterC.add(chanCSample);

        if ((main::currentTact % (CLOCK_DIVISOR * CLOCK_RATIO)) != 0)
        {
            return;
        }

        for (int i = 0; i < 3; ++i)
        {
            toneLevel[i] = toneLevels[registers[8 + i] & 15];
        }

        envshape = registers[13];
        level = toneLevels[envCounter];

        for (int i = 0; i < 3; ++i)
        {
            if (registers[8 + i] & 16)
            {
                toneLevel[i] = level;
            }
        }

        envCycles += CLOCK_DIVISOR;
        noiseCount = 0;
        while (envCycles >= 16)
        {
            envCycles -= 16;
            ++noiseCount;
            ++envTick;

            while (envTick >= envPeriod)
            {
                envTick -= envPeriod;

                if (envFirst || ((envshape & ENV_CONT) && !(envshape & ENV_HOLD)))
                {
                    if (envRev)
                    {
                        envCounter -= (envshape & ENV_ATTACK) ? 1 : -1;
                    }
                    else
                    {
                        envCounter += (envshape & ENV_ATTACK) ? 1 : -1;
                        if (envCounter < 0)
                        {
                            envCounter = 0;
                        }
                        if (envCounter > 15)
                        {
                            envCounter = 15;
                        }
                    }
                }

                ++envInternalTick;
                while (envInternalTick >= 16)
                {
                    envInternalTick -= 16;

                    if (!(envshape & ENV_CONT))
                    {
                        envCounter = 0;
                    }
                    else
                    {
                        if (envshape & ENV_HOLD)
                        {
                            if (envFirst && (envshape & ENV_ALT))
                            {
                                envCounter = (envCounter ? 0 : 15);
                            }
                        }
                        else
                        {
                            if (envshape & ENV_ALT)
                            {
                                envRev = !envRev;
                            }
                            else
                            {
                                envCounter = (envshape & ENV_ATTACK) ? 0 : 15;
                            }
                        }
                    }

                    envFirst = 0;
                }

                if (!envPeriod)
                {
                    break;
                }
            }
        }

        chanASample = toneLevel[0];
        chanBSample = toneLevel[1];
        chanCSample = toneLevel[2];
        mixer = registers[7];

        toneCycles += CLOCK_DIVISOR;
        toneCount = toneCycles >> 3;
        toneCycles &= 7;

        if ((mixer & 1) == 0)
        {
            level = chanASample;
            doTone(level, toneCount, &chanASample, 0);
        }
        if ((mixer & 0x08) == 0 && noiseToggle)
        {
            chanASample = 0;
        }

        if ((mixer & 2) == 0)
        {
            level = chanBSample;
            doTone(level, toneCount, &chanBSample, 1);
        }
        if ((mixer & 0x10) == 0 && noiseToggle)
        {
            chanBSample = 0;
        }

        if ((mixer & 4) == 0)
        {
            level = chanCSample;
            doTone(level, toneCount, &chanCSample, 2);
        }
        if ((mixer & 0x20) == 0 && noiseToggle)
        {
            chanCSample = 0;
        }

        noiseTick += noiseCount;
        while (noiseTick >= noisePeriod)
        {
            noiseTick -= noisePeriod;

            if ((rng & 1) ^ ((rng & 2) ? 1 : 0))
            {
                noiseToggle = !noiseToggle;
            }

            if (rng & 1)
            {
                rng ^= 0x24000;
            }
            rng >>= 1;

            if (!noisePeriod)
            {
                break;
            }
        }
    }
}