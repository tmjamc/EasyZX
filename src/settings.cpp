#include "settings.h"
#include "ini.h"

// Handles persistent emulator configuration.
// Settings are loaded from an INI file during startup
// and written back when the application exits or the
// user changes configuration.
namespace settings
{
    // Global runtime configuration currently used by the emulator.
    Settings current;
    
    // Load an integer value if the key exists.
    // Missing keys leave the current value unchanged.
    static void loadInt(int &variable, const mINI::INIMap<std::string> &section, const std::string &key)
    {
        const std::string value = section.get(key);
        variable = value.empty() ? variable : std::stoi(value);
    }
    
    // Load a boolean value if the key exists.
    // Missing keys leave the current value unchanged.
    static void loadBool(bool &variable, const mINI::INIMap<std::string> &section, const std::string &key)
    {
        const std::string value = section.get(key);
        variable = value.empty() ? variable : (std::stoi(value) != 0);
    }
    
    // Load a string value if the key exists.
    // Missing keys leave the current value unchanged.
    static void loadString(std::string &variable, const mINI::INIMap<std::string> &section, const std::string &key)
    {
        const std::string value = section.get(key);
        variable = value.empty() ? variable : value;
    }
    
    // Load a float value if the key exists.
    // Missing keys leave the current value unchanged.
    static void loadFloat(float &variable, const mINI::INIMap<std::string> &section, const std::string &key)
    {
        const std::string value = section.get(key);
        variable = value.empty() ? variable : std::stof(value);
    }

    // Load settings from INI file.
    // Existing values remain untouched when a key is missing.
    void load()
    {
        current = {};
        
        mINI::INIFile file(settingsFileName);

        mINI::INIStructure ini;

        if (!file.read(ini))
        {
            return;
        }
        
        // Display settings
        const auto &display = ini["display"];
        loadInt(current.displayHorizontalBorderSize, display, "horizontalBorderSize");
        loadInt(current.displayVerticalBorderSize, display, "verticalBorderSize");
        loadFloat(current.displayPixelRatio, display, "pixelRatio");
        loadInt(current.displayShaderIndex, display, "shaderIndex");
        loadInt(current.displayGigaScreenMode, display, "gigaScreenMode");
        loadBool(current.displayDarkTheme, display, "darkTheme");
        loadFloat(current.displayBackgroundColorR, display, "backgroundColorR");
        loadFloat(current.displayBackgroundColorG, display, "backgroundColorG");
        loadFloat(current.displayBackgroundColorB, display, "backgroundColorB");
        loadBool(current.displayFullScreen, display, "fullScreen");
        loadBool(current.displayShowRefreshRatePopup, display, "showRefreshRatePopup");

        // Model settings
        const auto &model = ini["model"];
        loadInt(current.modelId, model, "id");

        // Tape settings
        const auto &tape = ini["tape"];
        loadBool(current.tapeAutoStartStop, tape, "autoStartStop");
        loadBool(current.tapeThrottleLoading, tape, "throttleLoading");
        loadBool(current.tapeInstantLoading, tape, "instantLoading");

        // Audio settings
        const auto &audio = ini["audio"];
        loadInt(current.audioDeviceId, audio, "deviceId");
        loadInt(current.audioAyVolume, audio, "ayVolume");
        loadInt(current.audioBeeperVolume, audio, "beeperVolume");
        loadInt(current.audioTapeVolume, audio, "tapeVolume");
        loadInt(current.audioAyDcAdjustBufferLength, audio, "ayDcAdjustBufferLength");
        loadInt(current.audioBeeperDcAdjustBufferLength, audio, "beeperDcAdjustBufferLength");
        loadInt(current.audioTapeDcAdjustBufferLength, audio, "tapeDcAdjustBufferLength");

        // Main window settings
        const auto &windowMain = ini["windowMain"];
        loadInt(current.windowMainLeft, windowMain, "left");
        loadInt(current.windowMainTop, windowMain, "top");
        loadInt(current.windowMainWidth, windowMain, "width");
        loadInt(current.windowMainHeight, windowMain, "height");
        loadInt(current.windowMainStatus, windowMain, "status");

        // Devices settings
        const auto &devices = ini["devices"];
        loadBool(current.devicesBetaDisk, devices, "betaDisk");
        loadBool(current.devicesAY, devices, "ay");
    }

    // Serialize the current runtime configuration into
    // an INI file so settings persist between sessions.
    void save()
    {
        mINI::INIFile file(settingsFileName);

        mINI::INIStructure ini;
        
        // Display settings
        ini["display"].set(
            {{"horizontalBorderSize", std::to_string(current.displayHorizontalBorderSize)},
             {"verticalBorderSize", std::to_string(current.displayVerticalBorderSize)},
             {"pixelRatio", std::to_string(current.displayPixelRatio)},
             {"shaderIndex", std::to_string(current.displayShaderIndex)},
             {"gigaScreenMode", std::to_string(current.displayGigaScreenMode)},
             {"darkTheme", std::to_string(current.displayDarkTheme)},
             {"backgroundColorR", std::to_string(current.displayBackgroundColorR)},
             {"backgroundColorG", std::to_string(current.displayBackgroundColorG)},
             {"backgroundColorB", std::to_string(current.displayBackgroundColorB)},
             {"fullScreen", std::to_string(current.displayFullScreen)},
             {"showRefreshRatePopup", std::to_string(current.displayShowRefreshRatePopup)}});
            
        // Model settings
        ini["model"].set(
            {{"id", std::to_string(current.modelId)}});

        // Tape settings
        ini["tape"].set( 
            {{"autoStartStop", std::to_string(current.tapeAutoStartStop)},
             {"throttleLoading", std::to_string(current.tapeThrottleLoading)},
             {"instantLoading", std::to_string(current.tapeInstantLoading)}});

        // Audio settings
        ini["audio"].set(
            {{"deviceId", std::to_string(current.audioDeviceId)},
             {"ayVolume", std::to_string(current.audioAyVolume)},
             {"beeperVolume", std::to_string(current.audioBeeperVolume)},
             {"tapeVolume", std::to_string(current.audioTapeVolume)},
             {"ayDcAdjustBufferLength", std::to_string(current.audioAyDcAdjustBufferLength)},
             {"beeperDcAdjustBufferLength", std::to_string(current.audioBeeperDcAdjustBufferLength)},
             {"tapeDcAdjustBufferLength", std::to_string(current.audioTapeDcAdjustBufferLength)}});

        // Main window settings
        ini["windowMain"].set(
            {{"left", std::to_string(current.windowMainLeft)},
             {"top", std::to_string(current.windowMainTop)},
             {"width", std::to_string(current.windowMainWidth)},
             {"height", std::to_string(current.windowMainHeight)},
             {"status", std::to_string(current.windowMainStatus)}});

        // Devices settings
        ini["devices"].set(
            {{"betaDisk", std::to_string(current.devicesBetaDisk)},
             {"ay", std::to_string(current.devicesAY)}});

        file.generate(ini, true);
    }
}