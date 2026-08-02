#pragma once

#include <unordered_set>

namespace file_browser
{
    struct ExtensionFilter
    {
        std::string description;
        std::unordered_set<std::string> extensions;
    };

    extern std::vector<ExtensionFilter> tapeFilters;

    void open(std::string path, std::vector<ExtensionFilter> filter);
    void render();
}