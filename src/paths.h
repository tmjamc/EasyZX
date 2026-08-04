#pragma once

#include <filesystem>
#include <string>
#include <shlobj.h>

namespace paths
{
    extern std::filesystem::path settingsPath;
    extern std::filesystem::path fileBrowserPath;

    void init();

    // Preferred API: keeps the OS-native path representation intact.
    std::filesystem::path getKnownFolderPath(REFKNOWNFOLDERID folderId);

    // Backward-compatible API for existing callers that expect UTF-8 text.
    std::string getFolder(REFKNOWNFOLDERID folderId);
}
