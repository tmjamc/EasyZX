#include <windows.h>

#include "paths.h"

namespace paths
{
    namespace
    {
        constexpr const char* APPLICATION_FOLDER_NAME = "EasyZX";
        constexpr const char* SETTINGS_FILE_NAME = "settings.ini";
        constexpr const char* FILE_BROWSER_FILE_NAME = "file_browser.ini";

        std::string wideToString(LPWSTR wstr)
        {
            if (!wstr)
            {
                return std::string();
            }

            int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);

            if (sizeNeeded <= 0)
            {
                return std::string();
            }

            std::string result(sizeNeeded - 1, 0);
            WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &result[0], sizeNeeded, nullptr, nullptr);

            return result;
        }
    }

    std::filesystem::path settingsPath;
    std::filesystem::path fileBrowserPath;

    void init()
    {
        std::filesystem::path applicationFolderPath = std::filesystem::path(paths::getFolder(FOLDERID_RoamingAppData)).append(APPLICATION_FOLDER_NAME);
        std::filesystem::create_directories(applicationFolderPath);
        settingsPath = applicationFolderPath;
        settingsPath.append(SETTINGS_FILE_NAME);
        fileBrowserPath = applicationFolderPath;
        fileBrowserPath.append(FILE_BROWSER_FILE_NAME);
    }

    std::string getFolder(REFKNOWNFOLDERID folderId)
    {
        LPWSTR wszPath = nullptr;
        HRESULT hr;
        hr = SHGetKnownFolderPath(folderId, KF_FLAG_CREATE, NULL, &wszPath);
        
        std::string result{};
        
        if (SUCCEEDED(hr))
        {
            result = wideToString(wszPath);
        }

        CoTaskMemFree(wszPath);

        return result;
    }
}