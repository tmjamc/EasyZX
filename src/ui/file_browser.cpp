#include <iostream>
#include <filesystem>

#include "imgui.h"
#include "file_browser.h"

namespace file_browser
{
    namespace
    {
        std::string currentPath;
        std::unordered_set<std::string> currentFilter;
        std::vector<std::filesystem::directory_entry> entries;

        bool openRequest = false;
        bool opened = false;

        void updateEntries()
        {
            entries.clear();

            for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(currentPath))
            {
                if (entry.is_directory())
                {
                    entries.push_back(entry);
                    continue;
                }

                std::string extension = entry.path().extension().string();

                if (currentFilter.contains(extension))
                {
                    entries.push_back(entry);
                }
            }
        }
    }

    void open(std::string path, std::unordered_set<std::string> filter)
    {
        currentPath = path;
        currentFilter = filter;
        updateEntries();
        openRequest = true;
    }

    void render()
    {
        if (openRequest)
        {
            openRequest = false;
            opened = true;
            ImGui::OpenPopup("###file_browser_dialog");
        }

        if (!opened)
        {
            return;
        }

        if (ImGui::BeginPopupModal("###file_browser_dialog", &opened))
        {
            ImGui::Text(currentPath.c_str());

            if (ImGui::BeginTable("###entries_table", 3, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Date", ImGuiTableColumnFlags_WidthFixed, 16.0f);
                ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 110.0f);
                ImGui::TableSetupScrollFreeze(3, 1);
                ImGui::TableHeadersRow();
                
                for (int i = 0; i < entries.size(); ++i)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text(entries[i].path().filename().string().c_str());
                }

                ImGui::EndTable();
            }

            ImGui::EndPopup();
        }
    }
}