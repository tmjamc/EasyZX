#pragma once

#include <unordered_set>

namespace file_browser
{
    void open(std::string path, std::unordered_set<std::string> filter);
    void render();
}