#include <iostream>
#include <filesystem>
#include <ranges>

#include "zx_theme.h"
#include "file_browser.h"

namespace file_browser
{
    namespace
    {
        constexpr ImGuiSelectableFlags SELECTABLE_FLAGS = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick;

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

        int selectedRowIndex = -1;

        bool updateRequest = false;
        bool openRequest = false;
        bool opened = false;

        void updateEntries()
        {
            entries.clear();
            filteredAndSortedEntries.clear();

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

            selectedRowIndex = -1;
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
            return std::format("{} {}{}", std::ceil(mantissa * 10.0) / 10.0, i["BKMGTPE"], i ? "B" : "");
        }
    }

    void open(std::string path, std::unordered_set<std::string> filter)
    {
        currentPath = path;
        currentFilter = filter;
        openRequest = true;
        updateRequest = true;
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

        if (updateRequest)
        {
            updateRequest = false;
            updateEntries();
        }

        ImGui::SetNextWindowSize(ImVec2(800.0f, 600.0f));
        if (ImGui::BeginPopupModal("###file_browser_dialog", &opened))
        {
            ImGui::Text(currentPath.c_str());

            ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.35f, 0.35f, 0.35f, 0.40f));
            ImGui::PushStyleColor(ImGuiCol_SeparatorActive, ImVec4(0.35f, 0.35f, 0.35f, 0.40f));
            ImGui::PushStyleColor(ImGuiCol_SeparatorHovered, ImVec4(0.35f, 0.35f, 0.35f, 0.40f));

            if (ImGui::BeginTable("###entries_table", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Sortable | ImGuiTableFlags_Hideable | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_DefaultSort);
                ImGui::TableSetupColumn("Size");
                ImGui::TableSetupColumn("Date");
                ImGui::TableSetupScrollFreeze(0, 1);

                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 0.0f, 3.5f }); // Increase vertical padding
                ImGui::TableHeadersRow();
                ImGui::PopStyleVar();

                // Sorting
                ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();
                if (filteredAndSortedEntries.empty() || sortSpecs->SpecsDirty)
                {
                    // Sort your underlying data here
                    updateFilterAndSort(/*sortSpecs*/);
                    sortSpecs->SpecsDirty = false;
                }
                
                for (int rowIndex = 0; rowIndex < filteredAndSortedEntries.size(); ++rowIndex)
                {
                    ImGui::PushID(rowIndex);

                    ImGui::TableNextRow(ImGuiTableRowFlags_None, 24.0f);

                    ImGui::TableSetColumnIndex(0);
                    ImGui::ZXIcon(filteredAndSortedEntries[rowIndex].iconId);
                    ImGui::Text(filteredAndSortedEntries[rowIndex].directoryEntry.path().filename().string().c_str());
                    ImGui::SameLine();
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
                    const bool item_is_selected = rowIndex == selectedRowIndex;
                    if (ImGui::Selectable("", item_is_selected, SELECTABLE_FLAGS, ImVec2(0.0f, 14.0f)))
                    {
                        selectedRowIndex = rowIndex;
                        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        {
                            if (filteredAndSortedEntries[rowIndex].directoryEntry.is_directory())
                            {
                                currentPath = filteredAndSortedEntries[rowIndex].directoryEntry.path().string();
                                updateRequest = true;
                            }
                        }
                    }
                    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text(filteredAndSortedEntries[rowIndex].directoryEntry.is_regular_file() ? formatFileSize(filteredAndSortedEntries[rowIndex].directoryEntry.file_size()).c_str() : "");

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text(std::format(locale, "{:L%c}", filteredAndSortedEntries[rowIndex].directoryEntry.last_write_time()).c_str());

                    ImGui::PopID();
                }

                ImGui::EndTable();
            }

            ImGui::PopStyleColor(3);

            ImGui::EndPopup();
        }
    }
}