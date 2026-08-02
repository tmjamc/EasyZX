#pragma once

#include <string>
#include <filesystem>
#include <shlobj.h>

namespace paths
{
    extern std::filesystem::path settingsPath;
    extern std::filesystem::path fileBrowserPath;

    void init();
    std::string getFolder(REFKNOWNFOLDERID folderId);
}