#include "settings.h"
#include "ini.h"

// Handles persistent emulator configuration.
// Settings are loaded from an INI file during startup
// and written back when the application exits or the
// user changes configuration.
namespace settings
{
    constexpr static const char* SETTINGS_FILE_NAME = "./EasyZX.ini";

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
        
        mINI::INIFile file(SETTINGS_FILE_NAME);

        mINI::INIStructure ini;

        if (!file.read(ini))
        {
            return;
        }
        
        // Display settings
        const auto &display = ini["display"];
        loadInt(current.displayHorizontalBorderSize, display, "horizontal_border_size");
        loadInt(current.displayVerticalBorderSize, display, "vertical_border_size");
        loadFloat(current.displayPixelRatio, display, "pixel_ratio");
        loadInt(current.displayShaderIndex, display, "shader_index");
        loadInt(current.displayGigaScreenMode, display, "giga_screen_mode");
        loadBool(current.displayDarkTheme, display, "dark_theme");
        loadFloat(current.displayBackgroundColorR, display, "background_color_r");
        loadFloat(current.displayBackgroundColorG, display, "background_color_g");
        loadFloat(current.displayBackgroundColorB, display, "background_color_b");
        loadBool(current.displayFullScreen, display, "full_screen");
        loadBool(current.displayShowRefreshRatePopup, display, "show_refresh_rate_popup");

        // Model settings
        const auto &model = ini["model"];
        loadInt(current.modelId, model, "id");

        // Tape settings
        const auto &tape = ini["tape"];
        loadBool(current.tapeAutoStartStop, tape, "auto_start_stop");
        loadBool(current.tapeThrottleLoading, tape, "throttle_loading");
        loadBool(current.tapeInstantLoading, tape, "instant_loading");

        // Audio settings
        const auto &audio = ini["audio"];
        loadInt(current.audioDeviceId, audio, "device_id");
        loadInt(current.audioAyVolume, audio, "ay_volume");
        loadInt(current.audioBeeperVolume, audio, "beeper_volume");
        loadInt(current.audioTapeVolume, audio, "tape_volume");
        loadInt(current.audioAyDcAdjustBufferLength, audio, "ay_dc_adjust_buffer_length");
        loadInt(current.audioBeeperDcAdjustBufferLength, audio, "beeper_dc_adjust_buffer_length");
        loadInt(current.audioTapeDcAdjustBufferLength, audio, "tape_dc_adjust_buffer_length");

        // Main window settings
        const auto &windowMain = ini["window_main"];
        loadInt(current.windowMainLeft, windowMain, "left");
        loadInt(current.windowMainTop, windowMain, "top");
        loadInt(current.windowMainWidth, windowMain, "width");
        loadInt(current.windowMainHeight, windowMain, "height");
        loadInt(current.windowMainStatus, windowMain, "status");

        // Devices settings
        const auto &devices = ini["devices"];
        loadBool(current.devicesBetaDisk, devices, "beta_disk");
        loadBool(current.devicesAY, devices, "ay");
    }

    // Serialize the current runtime configuration into
    // an INI file so settings persist between sessions.
    void save()
    {
        mINI::INIFile file(SETTINGS_FILE_NAME);

        mINI::INIStructure ini;
        
        // Display settings
        ini["display"].set(
            {{"horizontal_border_size", std::to_string(current.displayHorizontalBorderSize)},
             {"vertical_border_size", std::to_string(current.displayVerticalBorderSize)},
             {"pixel_ratio", std::to_string(current.displayPixelRatio)},
             {"shader_index", std::to_string(current.displayShaderIndex)},
             {"giga_screen_mode", std::to_string(current.displayGigaScreenMode)},
             {"dark_theme", std::to_string(current.displayDarkTheme)},
             {"background_color_r", std::to_string(current.displayBackgroundColorR)},
             {"background_color_g", std::to_string(current.displayBackgroundColorG)},
             {"background_color_b", std::to_string(current.displayBackgroundColorB)},
             {"full_screen", std::to_string(current.displayFullScreen)},
             {"show_refresh_rate_popup", std::to_string(current.displayShowRefreshRatePopup)}});
            
        // Model settings
        ini["model"].set(
            {{"id", std::to_string(current.modelId)}});

        // Tape settings
        ini["tape"].set( 
            {{"auto_start_stop", std::to_string(current.tapeAutoStartStop)},
             {"throttle_loading", std::to_string(current.tapeThrottleLoading)},
             {"instant_loading", std::to_string(current.tapeInstantLoading)}});

        // Audio settings
        ini["audio"].set(
            {{"device_id", std::to_string(current.audioDeviceId)},
             {"ay_volume", std::to_string(current.audioAyVolume)},
             {"beeper_volume", std::to_string(current.audioBeeperVolume)},
             {"tape_volume", std::to_string(current.audioTapeVolume)},
             {"ay_dc_adjust_buffer_length", std::to_string(current.audioAyDcAdjustBufferLength)},
             {"beeper_dc_adjust_buffer_length", std::to_string(current.audioBeeperDcAdjustBufferLength)},
             {"tape_dc_adjust_buffer_length", std::to_string(current.audioTapeDcAdjustBufferLength)}});

        // Main window settings
        ini["window_main"].set(
            {{"left", std::to_string(current.windowMainLeft)},
             {"top", std::to_string(current.windowMainTop)},
             {"width", std::to_string(current.windowMainWidth)},
             {"height", std::to_string(current.windowMainHeight)},
             {"status", std::to_string(current.windowMainStatus)}});

        // Devices settings
        ini["devices"].set(
            {{"beta_disk", std::to_string(current.devicesBetaDisk)},
             {"ay", std::to_string(current.devicesAY)}});

        file.generate(ini, true);
    }
}