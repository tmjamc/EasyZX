#include <windows.h>
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
        constexpr float SPLITTER_WIDTH = 6.0f;
        constexpr float MIN_PANE_WIDTH = 100.0f;

        const std::locale &locale = std::locale("");

        struct FileEntry
        {
            std::filesystem::directory_entry directoryEntry;
            std::string extension;
            ImGui::ZXIconId iconId;
        };

        std::filesystem::path currentPath;
        std::unordered_set<std::string> currentFilter;
        std::vector<FileEntry> entries;
        std::vector<FileEntry> filteredAndSortedEntries;

        int selectedRowIndex = -1;
        int sortColumnIndex = 0;
        bool sortAscending = true;

        bool updateRequest = false;
        bool openRequest = false;
        bool opened = false;

        float leftPaneWidth = 250.0f;

        struct VolumeEntry
        {
            uint32_t type;
            std::string name;
        };

        std::vector<VolumeEntry> volumeEntries;

        void updateVolumeEntries()
        {
            volumeEntries.clear();
            DWORD drives = GetLogicalDrives();

            for (int i = 0; i < 26; ++i)
            {
                if (!(drives & (1 << i)))
                {
                    continue;
                }

                std::string root = std::string(1, 'A' + i) + ":\\";
                UINT type = GetDriveTypeA(root.c_str());
                std::string typeStr;
                switch (type)
                {
                    case DRIVE_REMOVABLE: typeStr = "USB Drive";  break;
                    case DRIVE_FIXED:     typeStr = "Local Disk"; break;
                    case DRIVE_REMOTE:    typeStr = "Network";    break;
                    case DRIVE_CDROM:     typeStr = "CD-ROM";     break;
                    case DRIVE_RAMDISK:   typeStr = "RAM disk";   break;
                    default:              typeStr = "Unknown";    break;
                }                

                char volumeName[MAX_PATH + 1] = {0};

                BOOL ok = GetVolumeInformationA(
                    root.c_str(),
                    volumeName, MAX_PATH,
                    nullptr, nullptr, nullptr, nullptr, MAX_PATH
                    // &serialNumber, &maxCompLen, &fsFlags,
                    // fsName, MAX_PATH
                );

                if (ok)
                {
                    volumeEntries.emplace_back(type, std::format("{} ({})", volumeName[0] ? volumeName : typeStr, root));
                }
            }
        }

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
                
                switch (sortColumnIndex)
                {
                case 0:
                {
                    std::string file1 = entry1.directoryEntry.path().filename().string();
                    std::transform(file1.begin(), file1.end(), file1.begin(), [](unsigned char c) { return std::tolower(c); });

                    std::string file2 = entry2.directoryEntry.path().filename().string();
                    std::transform(file2.begin(), file2.end(), file2.begin(), [](unsigned char c) { return std::tolower(c); });

                    return sortAscending ? file1 < file2 : file1 > file2;
                }
                case 1:
                {
                    return sortAscending ? entry1.directoryEntry.file_size() < entry2.directoryEntry.file_size() : entry1.directoryEntry.file_size() > entry2.directoryEntry.file_size();
                }
                case 2:
                {
                    return sortAscending ? entry1.directoryEntry.last_write_time() < entry2.directoryEntry.last_write_time() : entry1.directoryEntry.last_write_time() > entry2.directoryEntry.last_write_time();
                }
                }

                return false;
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

        updateVolumeEntries();
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
            // Available area inside the window
            ImVec2 available = ImGui::GetContentRegionAvail();

            float rightPaneWidth = available.x - leftPaneWidth - SPLITTER_WIDTH;

            // Clamp sizes
            if (leftPaneWidth < MIN_PANE_WIDTH)
            {
                leftPaneWidth = MIN_PANE_WIDTH;
            }

            if (rightPaneWidth < MIN_PANE_WIDTH)
            {
                rightPaneWidth = MIN_PANE_WIDTH;
                leftPaneWidth = available.x - SPLITTER_WIDTH - MIN_PANE_WIDTH;
            }

            // Left Pane
            ImGui::BeginChild("###left_pane", ImVec2(leftPaneWidth, 0), ImGuiChildFlags_None);

            // Volumes
            if (ImGui::BeginTable("###volumes", 1, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn("Volumes", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 0.0f, 3.5f });
                ImGui::TableHeadersRow();
                ImGui::PopStyleVar();

                for (const VolumeEntry &volumeEntry : volumeEntries)
                {
                    ImGui::TableNextRow(ImGuiTableRowFlags_None, 24.0f);
                    ImGui::TableSetColumnIndex(0);

                    ImGui::ZXIcon(ImGui::ZXIconId::FOLDER);
                    ImGui::TextUnformatted(volumeEntry.name.c_str());
                    ImGui::SameLine();
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);

                    if (ImGui::Selectable("", false, SELECTABLE_FLAGS, ImVec2(0.0f, 14.0f)))
                    {

                    }
                }

                ImGui::EndTable();
            }

            // TODO: Recent

            ImGui::EndChild();

            ImGui::SameLine();

            // Splitter
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 9.0f);
            ImGui::InvisibleButton("###splitter", ImVec2(SPLITTER_WIDTH, available.y));

            bool hovered = ImGui::IsItemHovered();
            bool active = ImGui::IsItemActive();
            if (hovered || active)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            }

            if (active)
            {
                leftPaneWidth += ImGui::GetIO().MouseDelta.x;
                if (leftPaneWidth < MIN_PANE_WIDTH)
                {
                    leftPaneWidth = MIN_PANE_WIDTH;
                }
            }

            // Draw splitter
            ImU32 color = active ? IM_COL32(180, 180, 180, 255) : (hovered ? IM_COL32(140, 140, 140, 255) : IM_COL32(0, 0, 0, 0));
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), color);
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10.0f);

            // Right Pane
            ImGui::BeginChild("###right_pane", ImVec2(0, 0), ImGuiChildFlags_None);

            ImGui::AlignTextToFramePadding();

            if (ImGui::Button("Up"))
            {
                currentPath = currentPath.parent_path();
                updateRequest = true;
            }
            ImGui::SameLine();

            std::string partialPath{};
            for (const auto& part : currentPath)
            {
                if (part == "\\" || part == "")
                {
                    continue;
                }

                partialPath = std::format("{}{}\\", partialPath, part.string());
                ImGui::SameLine();
                if (ImGui::TextLink(part.string().c_str()))
                {
                    currentPath = partialPath;
                    updateRequest = true;
                    break;
                }
                ImGui::SameLine();
                ImGui::TextUnformatted("\\");
            }

            if (updateRequest)
            {
                updateRequest = false;
                updateEntries();
            }

            ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.35f, 0.35f, 0.35f, 0.40f));
            ImGui::PushStyleColor(ImGuiCol_SeparatorActive, ImVec4(0.35f, 0.35f, 0.35f, 0.40f));
            ImGui::PushStyleColor(ImGuiCol_SeparatorHovered, ImVec4(0.35f, 0.35f, 0.35f, 0.40f));

            if (ImGui::BeginTable("###entries_table", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Sortable | ImGuiTableFlags_Hideable))
            {
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_DefaultSort);
                ImGui::TableSetupColumn("Size");
                ImGui::TableSetupColumn("Date");
                ImGui::TableSetupScrollFreeze(0, 1);

                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 0.0f, 3.5f });
                ImGui::TableHeadersRow();
                ImGui::PopStyleVar();

                ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();
                if (filteredAndSortedEntries.empty() || sortSpecs->SpecsDirty)
                {
                    sortColumnIndex = 0;
                    sortAscending = true;
                    for (int i = 0;  i < sortSpecs->SpecsCount; ++i)
                    {
                        if (sortSpecs->Specs[i].SortOrder == 0)
                        {
                            sortColumnIndex = sortSpecs->Specs[i].ColumnIndex;
                            sortAscending = sortSpecs->Specs[i].SortDirection == ImGuiSortDirection::ImGuiSortDirection_Ascending;
                            break;
                        }
                    }
                    updateFilterAndSort();
                    sortSpecs->SpecsDirty = false;
                }
                
                for (int rowIndex = 0; rowIndex < filteredAndSortedEntries.size(); ++rowIndex)
                {
                    ImGui::PushID(rowIndex);

                    ImGui::TableNextRow(ImGuiTableRowFlags_None, 24.0f);

                    ImGui::TableSetColumnIndex(0);
                    ImGui::ZXIcon(filteredAndSortedEntries[rowIndex].iconId);
                    ImGui::TextUnformatted(filteredAndSortedEntries[rowIndex].directoryEntry.path().filename().string().c_str());
                    ImGui::SameLine();
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
                    if (ImGui::Selectable("", rowIndex == selectedRowIndex, SELECTABLE_FLAGS, ImVec2(0.0f, 14.0f)))
                    {
                        selectedRowIndex = rowIndex;
                        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        {
                            if (filteredAndSortedEntries[rowIndex].directoryEntry.is_directory())
                            {
                                currentPath = filteredAndSortedEntries[rowIndex].directoryEntry.path();
                                updateRequest = true;
                            }
                        }
                    }
                    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(filteredAndSortedEntries[rowIndex].directoryEntry.is_regular_file() ? formatFileSize(filteredAndSortedEntries[rowIndex].directoryEntry.file_size()).c_str() : "");

                    ImGui::TableSetColumnIndex(2);
                    ImGui::TextUnformatted(std::format(locale, "{:L%c}", filteredAndSortedEntries[rowIndex].directoryEntry.last_write_time()).c_str());

                    ImGui::PopID();
                }

                ImGui::EndTable();
            }

            ImGui::PopStyleColor(3);

            ImGui::EndChild();

            ImGui::EndPopup();
        }
    }
}