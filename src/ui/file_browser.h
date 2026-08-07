#pragma once

#include <unordered_set>
#include <functional>
#include <string>

namespace file_browser
{
    struct ExtensionFilter
    {
        std::string description;
        std::unordered_set<std::string> extensions;
    };

    extern std::vector<ExtensionFilter> tapeFilters;
    extern std::vector<ExtensionFilter> diskFilters;

    void open(std::vector<ExtensionFilter> filter, std::function<void(const std::string&)> callBack);
    void render();
}