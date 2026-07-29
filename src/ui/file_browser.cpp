#include <iostream>
#include <filesystem>

#include "zx_theme.h"
#include "file_browser.h"

namespace file_browser
{
    namespace
    {
        const std::locale &locale = std::locale("");

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

        std::string formatFileSize(std::uintmax_t size)
        {
            double mantissa = size;
            int i = 0;
            while (mantissa >= 1024.0)
            {
                mantissa /= 1024.0;
                ++i;
            }
            return std::format("{}{}{}", std::ceil(mantissa * 10.0) / 10.0, i["BKMGTPE"], i ? "B" : "");
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

        ImGui::SetNextWindowSize(ImVec2(800.0f, 600.0f));
        if (ImGui::BeginPopupModal("###file_browser_dialog", &opened))
        {
            ImGui::Text(currentPath.c_str());

            ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.35f, 0.35f, 0.35f, 0.40f));
            ImGui::PushStyleColor(ImGuiCol_SeparatorActive, ImVec4(0.35f, 0.35f, 0.35f, 0.40f));
            ImGui::PushStyleColor(ImGuiCol_SeparatorHovered, ImVec4(0.35f, 0.35f, 0.35f, 0.40f));

            if (ImGui::BeginTable("###entries_table", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Sortable | ImGuiTableFlags_Hideable))
            {
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Size");
                ImGui::TableSetupColumn("Date");
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();
                
                for (int i = 0; i < entries.size(); ++i)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text(entries[i].path().filename().string().c_str());
                    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text(formatFileSize(entries[i].file_size()).c_str());

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text(std::format(locale, "{:L%c}", entries[i].last_write_time()).c_str());
                }

                ImGui::EndTable();
            }

            ImGui::PopStyleColor(3);

            ImGui::EndPopup();
        }
    }
}