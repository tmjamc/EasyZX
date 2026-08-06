#include "paths.h"

#include <windows.h>
#include <shlobj.h>

#include <filesystem>
#include <limits>
#include <string>
#include <string_view>
#include <system_error>

namespace paths
{
    std::filesystem::path settingsPath;
    std::filesystem::path fileBrowserPath;

    namespace
    {
        constexpr wchar_t APPLICATION_FOLDER_NAME[] = L"EasyZX";
        constexpr wchar_t SETTINGS_FILE_NAME[] = L"settings.ini";
        constexpr wchar_t FILE_BROWSER_FILE_NAME[] = L"file_browser.ini";

        struct CoTaskMemWString
        {
            PWSTR value = nullptr;

            ~CoTaskMemWString()
            {
                if (value != nullptr)
                {
                    CoTaskMemFree(value);
                    value = nullptr;
                }
            }

            CoTaskMemWString() = default;
            CoTaskMemWString(const CoTaskMemWString&) = delete;
            CoTaskMemWString& operator=(const CoTaskMemWString&) = delete;
        };

        std::string wideToUtf8(std::wstring_view text)
        {
            if (text.empty())
            {
                return {};
            }

            if (text.size() > static_cast<size_t>((std::numeric_limits<int>::max)()))
            {
                return {};
            }

            const int wideLength = static_cast<int>(text.size());
            const int bytesNeeded = WideCharToMultiByte(
                CP_UTF8,
                WC_ERR_INVALID_CHARS,
                text.data(),
                wideLength,
                nullptr,
                0,
                nullptr,
                nullptr);

            if (bytesNeeded <= 0)
            {
                return {};
            }

            std::string result(static_cast<size_t>(bytesNeeded), '\0');
            const int bytesWritten = WideCharToMultiByte(
                CP_UTF8,
                WC_ERR_INVALID_CHARS,
                text.data(),
                wideLength,
                result.data(),
                bytesNeeded,
                nullptr,
                nullptr);

            if (bytesWritten != bytesNeeded)
            {
                return {};
            }

            return result;
        }

        std::filesystem::path getApplicationFolder()
        {
            auto appDataFolder = getKnownFolderPath(FOLDERID_RoamingAppData);

            // Keep a deterministic fallback instead of accidentally creating a
            // relative "EasyZX" folder when the known-folder lookup fails.
            if (appDataFolder.empty())
            {
                std::error_code ec;
                appDataFolder = std::filesystem::current_path(ec);
                if (ec)
                {
                    appDataFolder.clear();
                }
            }

            return appDataFolder / APPLICATION_FOLDER_NAME;
        }
    }

    void init()
    {
        const auto applicationFolder = getApplicationFolder();

        std::error_code ec;
        std::filesystem::create_directories(applicationFolder, ec);

        settingsPath = applicationFolder / SETTINGS_FILE_NAME;
        fileBrowserPath = applicationFolder / FILE_BROWSER_FILE_NAME;
    }

    std::filesystem::path getKnownFolderPath(REFKNOWNFOLDERID folderId)
    {
        CoTaskMemWString knownFolder;
        const HRESULT hr = SHGetKnownFolderPath(folderId, KF_FLAG_CREATE, nullptr, &knownFolder.value);

        if (FAILED(hr) || knownFolder.value == nullptr)
        {
            return {};
        }

        return std::filesystem::path(knownFolder.value);
    }

    std::string getFolder(REFKNOWNFOLDERID folderId)
    {
        const auto folderPath = getKnownFolderPath(folderId);
        return wideToUtf8(folderPath.native());
    }
}
