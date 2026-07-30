#include <iostream>
#include <filesystem>
#include <ranges>

#include "zx_theme.h"
#include "file_browser.h"

namespace file_browser
{
    namespace
    {
        const std::locale &locale = std::locale("");

        struct FileEntry
        {
            std::filesystem::directory_entry directoryEntry;
            std::string extension;
            ImGui::ZXIconId iconId;
        };

        std::string currentPath;
        std::unordered_set<std::string> currentFilter;
        std::vector<FileEntry> entries;
        std::vector<FileEntry> filteredAndSortedEntries;

        bool openRequest = false;
        bool opened = false;

        void updateEntries()
        {
            entries.clear();

            for (const std::filesystem::directory_entry &directoryEntry : std::filesystem::directory_iterator(currentPath))
            {
                std::string extension{};
                ImGui::ZXIconId iconId = ImGui::ZXIconId::DEFAULT;
                if (directoryEntry.is_directory())
                {
                    iconId = ImGui::ZXIconId::FOLDER;
                }
                else
                {
                    extension = directoryEntry.path().extension().string();
                    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return std::tolower(c); });

                    if (extension == ".tap"|| extension == ".tzx")
                    {
                        iconId = ImGui::ZXIconId::TAPE;
                    }
                }

                entries.emplace_back(directoryEntry, extension, iconId);
            }
        }

        void updateFilterAndSort()
        {
            auto filteredView = entries | std::views::filter([](const FileEntry &entry)
            {
                if (entry.directoryEntry.is_directory())
                {
                    return true;
                }
                return currentFilter.size() ? currentFilter.contains(entry.extension) : true;
            });

            filteredAndSortedEntries = std::vector<FileEntry>(filteredView.begin(), filteredView.end());

            std::ranges::sort(filteredAndSortedEntries, [](const FileEntry &entry1, const FileEntry &entry2)
            {
                if (entry1.directoryEntry.is_directory() && !entry2.directoryEntry.is_directory())
                {
                    return true;
                }
                if (!entry1.directoryEntry.is_directory() && entry2.directoryEntry.is_directory())
                {
                    return false;
                }
                
                std::string file1 = entry1.directoryEntry.path().filename().string();
                std::transform(file1.begin(), file1.end(), file1.begin(), [](unsigned char c) { return std::tolower(c); });

                std::string file2 = entry2.directoryEntry.path().filename().string();
                std::transform(file2.begin(), file2.end(), file2.begin(), [](unsigned char c) { return std::tolower(c); });

                return file1 < file2;
            });
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
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_DefaultSort);
                ImGui::TableSetupColumn("Size");
                ImGui::TableSetupColumn("Date");
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();

                // Sorting
                if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs())
                {
                    if (sortSpecs->SpecsDirty)
                    {
                        // Sort your underlying data here
                        updateFilterAndSort(/*sortSpecs*/);
                        sortSpecs->SpecsDirty = false;
                    }
                }
                
                for (int i = 0; i < filteredAndSortedEntries.size(); ++i)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::ZXIcon(filteredAndSortedEntries[i].iconId);
                    ImGui::Text(filteredAndSortedEntries[i].directoryEntry.path().filename().string().c_str());
                    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text(filteredAndSortedEntries[i].directoryEntry.is_regular_file() ? formatFileSize(filteredAndSortedEntries[i].directoryEntry.file_size()).c_str() : "");

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text(std::format(locale, "{:L%c}", filteredAndSortedEntries[i].directoryEntry.last_write_time()).c_str());
                }

                ImGui::EndTable();
            }

            ImGui::PopStyleColor(3);

            ImGui::EndPopup();
        }
    }
}