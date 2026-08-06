#include <windows.h>
#include <shlobj.h>
#include <iostream>
#include <filesystem>
#include <ranges>

#include "zx_theme.h"
#include "file_browser.h"
#include "paths.h"
#include "ini.h"
#include "settings.h"

namespace file_browser
{
    namespace
    {
        constexpr float SPLITTER_WIDTH = 8.0f;
        constexpr float MIN_PANE_WIDTH = 100.0f;
        constexpr int FILE_NAME_LENGTH = 300;

        const std::locale &locale = std::locale("");

        std::filesystem::path currentPath = paths::getFolder(FOLDERID_Profile);

        struct FileEntry
        {
            std::filesystem::directory_entry directoryEntry;
            std::string extension;
            ImGui::ZXIconId iconId;
        };
        std::vector<FileEntry> entries;
        std::vector<FileEntry> filteredAndSortedEntries;

        std::vector<ExtensionFilter> extensionFilters;
        int selectedExtensionFilterIndex = 0;

        int selectedRowIndex = -1;
        int sortColumnIndex = 0;
        bool sortAscending = true;

        bool updateRequest = false;
        bool openRequest = false;
        bool opened = false;

        float leftPaneWidth = 200.0f;
        float column1Width = 60.0f;
        float column2Width = 120.0f;

        char fileName[FILE_NAME_LENGTH]{};

        struct VolumeEntry
        {
            uint32_t type;
            std::string path;
            std::string name;
        };
        std::vector<VolumeEntry> volumeEntries;

        struct FolderEntry
        {
            std::string path;
            std::string name;
        };        
        std::vector<FolderEntry> folderEntries;

        struct RecentEntry
        {
            int count;
            std::string path;
        };        
        std::vector<RecentEntry> recentEntries;

        ImVec2 size(800.0f, 600.0f);
        ImVec2 position((GetSystemMetrics(SM_CXSCREEN) - size.x) / 2.0f, (GetSystemMetrics(SM_CYSCREEN) - size.y) / 2.0f);
        
        std::function<void(const std::string&)> fileSelectedCallBack;

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
                );

                if (ok)
                {
                    volumeEntries.emplace_back(type, root, std::format("{} ({})", volumeName[0] ? volumeName : typeStr, root));
                }
            }

            folderEntries.clear();
            folderEntries.emplace_back(paths::getFolder(FOLDERID_Desktop), std::filesystem::path(paths::getFolder(FOLDERID_Desktop)).filename().string());
            folderEntries.emplace_back(paths::getFolder(FOLDERID_Downloads), std::filesystem::path(paths::getFolder(FOLDERID_Downloads)).filename().string());
            folderEntries.emplace_back(paths::getFolder(FOLDERID_Documents), std::filesystem::path(paths::getFolder(FOLDERID_Documents)).filename().string());
            folderEntries.emplace_back(paths::getFolder(FOLDERID_Pictures), std::filesystem::path(paths::getFolder(FOLDERID_Pictures)).filename().string());
            folderEntries.emplace_back(paths::getFolder(FOLDERID_Music), std::filesystem::path(paths::getFolder(FOLDERID_Music)).filename().string());
            folderEntries.emplace_back(paths::getFolder(FOLDERID_Videos), std::filesystem::path(paths::getFolder(FOLDERID_Videos)).filename().string());
        }

        void updateRecentEntries(std::filesystem::path pathToAdd = "")
        {
            if (!pathToAdd.empty())
            {
                bool found = false;
                std::string path = pathToAdd.parent_path().string();
                for (RecentEntry &entry : recentEntries)
                {
                    if (path.compare(entry.path) == 0)
                    {
                        found = true;
                        ++entry.count;
                        break;
                    }
                }

                if (!found)
                {
                    recentEntries.emplace_back(1, path);
                }
            }

            std::ranges::sort(recentEntries, [](const RecentEntry &entry1, const RecentEntry &entry2)
            {
                return entry1.count < entry2.count;
            });
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
                return extensionFilters[selectedExtensionFilterIndex].extensions.size() ? extensionFilters[selectedExtensionFilterIndex].extensions.contains(entry.extension) : true;
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
    
        void load()
        {
            mINI::INIFile file(paths::fileBrowserPath);

            mINI::INIStructure ini;

            if (!file.read(ini))
            {
                return;
            }

            const auto &dimensions = ini["dimensions"];
            settings::loadFloat(position.x, dimensions, "position.x");
            settings::loadFloat(position.y, dimensions, "position.y");
            settings::loadFloat(size.x, dimensions, "size.x");
            settings::loadFloat(size.y, dimensions, "size.y");
            
            const auto &layout = ini["layout"];
            settings::loadFloat(leftPaneWidth, layout, "left_pane_width");
            settings::loadFloat(column1Width, layout, "column_size_Width");
            settings::loadFloat(column2Width, layout, "column_date_width");
            settings::loadInt(sortColumnIndex, layout, "sort_column_index");
            settings::loadBool(sortAscending, layout, "sort_ascending");
            
            const auto &paths = ini["paths"];
            std::string value = currentPath.string();
            settings::loadString(value, paths, "current_path");
            currentPath = value;
            
            const auto &recent = ini["recent"];
            recentEntries.clear();
            int index = 0;
            // std::string value;
            while ((value = recent.get(std::format("w{:02d}", index++))) != "")
            {
                const int c = value.find_first_of(',');
                recentEntries.emplace_back(std::stoi(value.substr(0, c)), value.substr(c + 1));
            }
        }

        void save()
        {
            mINI::INIFile file(paths::fileBrowserPath);

            mINI::INIStructure ini;
            
            ini["dimensions"].set(
                {{"position.x", std::to_string(position.x)},
                 {"position.y", std::to_string(position.y)},
                 {"size.x", std::to_string(size.x)},
                 {"size.y", std::to_string(size.y)}});

            ini["layout"].set(
                {{"left_pane_width", std::to_string(leftPaneWidth)},
                 {"column_size_Width", std::to_string(column1Width)},
                 {"column_date_width", std::to_string(column2Width)},
                 {"sort_column_index", std::to_string(sortColumnIndex)},
                 {"sort_ascending", std::to_string(sortAscending)}});

            ini["paths"].set(
                {{"current_path", currentPath.string()}});

            auto &recent = ini["recent"];
            int index = 0;
            for (const RecentEntry &entry : recentEntries)
            {
                recent.set(std::format("w{:02d}", index++), std::format("{},{}", entry.count, entry.path));
            }

            file.generate(ini, true);
        }

        void selectFile()
        {
            std::filesystem::path filePath = std::filesystem::path(currentPath).append(fileName);

            if (std::filesystem::exists(filePath))
            {
                fileSelectedCallBack(filePath.string());
                updateRecentEntries(filePath);
                opened = false;
            }
        }
    }

    std::vector<ExtensionFilter> tapeFilters =
    {
        {"Tape files (*.tap, *.tzx)", {".tap", ".tzx"}},
        {"All files (*.*)", {}}
    };

    std::vector<ExtensionFilter> diskFilters =
    {
        {"Disk files (*.trd, *.scl)", {".trd", ".scl"}},
        {"All files (*.*)", {}}
    };

    void open(std::vector<ExtensionFilter> filter, std::function<void(const std::string&)> callBack)
    {
        load();
        extensionFilters = filter;
        fileSelectedCallBack = callBack;
        selectedExtensionFilterIndex = 0;
        openRequest = true;
        updateRequest = true;
        memset(fileName, 0, FILE_NAME_LENGTH);

        updateVolumeEntries();
        updateRecentEntries();
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

        ImGui::SetNextWindowPos(position, ImGuiCond_Once);
        ImGui::SetNextWindowSize(size, ImGuiCond_Once);
        if (ImGui::BeginPopupModal("###file_browser_dialog", &opened))
        {
            // Available area inside the window
            ImVec2 available = ImGui::GetContentRegionAvail();

            float rightPaneWidth = available.x - leftPaneWidth - SPLITTER_WIDTH;
            float panesHeight = available.y - 40.0f;

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
            ImGui::BeginChild("###left_pane", ImVec2(leftPaneWidth, panesHeight), ImGuiChildFlags_None);

            // Volumes
            if (ImGui::BeginTable("###volumes", 1, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn("Volumes", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 0.0f, 3.5f });
                ImGui::TableHeadersRow();
                ImGui::PopStyleVar();

                int rowIndex = 0;
                for (const VolumeEntry &volumeEntry : volumeEntries)
                {
                    ImGui::PushID(rowIndex++);

                    ImGui::TableNextRow(ImGuiTableRowFlags_None, 24.0f);
                    ImGui::TableSetColumnIndex(0);

                    ImGui::ZXIcon(ImGui::ZXIconId::FOLDER);
                    ImGui::TextUnformatted(volumeEntry.name.c_str());
                    ImGui::SameLine();
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);

                    if (ImGui::Selectable("", false, ImGuiSelectableFlags_SpanAllColumns, ImVec2(0.0f, 14.0f)))
                    {
                        currentPath = volumeEntry.path;
                        updateRequest = true;
                    }

                    ImGui::PopID();
                }

                ImGui::EndTable();
            }

            // Folders
            if (ImGui::BeginTable("###folders", 1, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn("Folders", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 0.0f, 3.5f });
                ImGui::TableHeadersRow();
                ImGui::PopStyleVar();

                int rowIndex = 0;
                for (const FolderEntry &folderEntry : folderEntries)
                {
                    ImGui::PushID(rowIndex++);

                    ImGui::TableNextRow(ImGuiTableRowFlags_None, 24.0f);
                    ImGui::TableSetColumnIndex(0);

                    ImGui::ZXIcon(ImGui::ZXIconId::FOLDER);
                    ImGui::TextUnformatted(folderEntry.name.c_str());
                    ImGui::SameLine();
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);

                    if (ImGui::Selectable("", false, ImGuiSelectableFlags_SpanAllColumns, ImVec2(0.0f, 14.0f)))
                    {
                        currentPath = folderEntry.path;
                        updateRequest = true;
                    }

                    ImGui::PopID();
                }

                ImGui::EndTable();
            }

            // Recent
            if (ImGui::BeginTable("###recent", 1, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn("Recent", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 0.0f, 3.5f });
                ImGui::TableHeadersRow();
                ImGui::PopStyleVar();

                int rowIndex = 0;
                for (const RecentEntry &entry : recentEntries)
                {
                    ImGui::PushID(rowIndex++);

                    ImGui::TableNextRow(ImGuiTableRowFlags_None, 24.0f);
                    ImGui::TableSetColumnIndex(0);

                    ImGui::ZXIcon(ImGui::ZXIconId::FOLDER);
                    auto fileName = std::filesystem::path(entry.path).filename();
                    if (fileName.empty())
                    {
                        fileName = std::filesystem::path(entry.path).remove_filename().filename();
                    }
                    ImGui::TextUnformatted(fileName.string().c_str());
                    ImGui::SameLine();
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);

                    if (ImGui::Selectable("", false, ImGuiSelectableFlags_SpanAllColumns, ImVec2(0.0f, 14.0f)))
                    {
                        currentPath = entry.path;
                        updateRequest = true;
                    }

                    ImGui::PopID();
                }

                ImGui::EndTable();
            }

            ImGui::EndChild();

            ImGui::SameLine();

            // Splitter
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 9.0f);
            ImGui::InvisibleButton("###splitter", ImVec2(SPLITTER_WIDTH, panesHeight));

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

            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10.0f);

            // Right Pane
            ImGui::BeginChild("###right_pane", ImVec2(0, panesHeight), ImGuiChildFlags_None);

            ImGui::AlignTextToFramePadding();

            if (ImGui::Button("Up"))
            {
                currentPath = currentPath.parent_path();
                updateRequest = true;
            }
            ImGui::SameLine();
            
            ImGui::AlignTextToFramePadding();

            ImGui::Dummy(ImVec2(0.0f, 0.0f));
            ImGui::SameLine();

            ImGui::AlignTextToFramePadding();
            
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
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
            ImGui::PopStyleVar();

            ImGui::Dummy(ImVec2(0.0f, 0.0f));

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
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch | (sortColumnIndex == 0 ? ImGuiTableColumnFlags_DefaultSort : ImGuiTableColumnFlags_None) | (sortAscending ? ImGuiTableColumnFlags_PreferSortAscending : ImGuiTableColumnFlags_PreferSortDescending));
                ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed | (sortColumnIndex == 1 ? ImGuiTableColumnFlags_DefaultSort : ImGuiTableColumnFlags_None) | (sortAscending ? ImGuiTableColumnFlags_PreferSortAscending : ImGuiTableColumnFlags_PreferSortDescending), column1Width);
                ImGui::TableSetupColumn("Date", ImGuiTableColumnFlags_WidthFixed | (sortColumnIndex == 2 ? ImGuiTableColumnFlags_DefaultSort : ImGuiTableColumnFlags_None) | (sortAscending ? ImGuiTableColumnFlags_PreferSortAscending : ImGuiTableColumnFlags_PreferSortDescending), column2Width);
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
                    if (ImGui::Selectable("", rowIndex == selectedRowIndex, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0.0f, 14.0f)))
                    {
                        selectedRowIndex = rowIndex;
                        if (filteredAndSortedEntries[rowIndex].directoryEntry.is_regular_file())
                        {
                            const std::string selectedFileName = filteredAndSortedEntries[rowIndex].directoryEntry.path().filename().string();
                            memcpy(fileName, selectedFileName.c_str(), selectedFileName.length());
                            memset(fileName + selectedFileName.length(), 0, 1);
                        }
                        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        {
                            if (filteredAndSortedEntries[rowIndex].directoryEntry.is_directory())
                            {
                                currentPath = filteredAndSortedEntries[rowIndex].directoryEntry.path();
                                updateRequest = true;
                            }
                            if (filteredAndSortedEntries[rowIndex].directoryEntry.is_regular_file())
                            {
                                selectFile();
                            }
                        }
                    }
                    
                    ImGui::TableSetColumnIndex(1);
                    column1Width = ImGui::GetColumnWidth();

                    const std::string sizeText = filteredAndSortedEntries[rowIndex].directoryEntry.is_regular_file() ? formatFileSize(filteredAndSortedEntries[rowIndex].directoryEntry.file_size()) : "";
                    const float posX = ImGui::GetCursorPosX() + column1Width - ImGui::CalcTextSize(sizeText.c_str()).x;

                    if (posX > ImGui::GetCursorPosX())
                    {
                        ImGui::SetCursorPosX(posX);
                    }

                    ImGui::TextUnformatted(sizeText.c_str());

                    ImGui::TableSetColumnIndex(2);
                    column2Width = ImGui::GetColumnWidth();

                    ImGui::TextUnformatted(std::format(locale, "{:L%c}", filteredAndSortedEntries[rowIndex].directoryEntry.last_write_time()).c_str());

                    ImGui::PopID();
                }

                ImGui::EndTable();
            }

            ImGui::PopStyleColor(3);

            ImGui::EndChild();

            // Bottom bar
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 350.0f);
            ImGui::InputText("", fileName, FILE_NAME_LENGTH);

            ImGui::SameLine();
            ImGui::SetNextItemWidth(200.0f);
            if (ImGui::BeginCombo("###extensions_filter", extensionFilters[selectedExtensionFilterIndex].description.c_str()))
            {
                for (int index = 0; index < extensionFilters.size(); ++index)
                {
                    const bool is_selected = (index == selectedExtensionFilterIndex);
                    if (ImGui::Selectable(extensionFilters[index].description.c_str(), is_selected))
                    {
                        if (index != selectedExtensionFilterIndex)
                        {
                            selectedExtensionFilterIndex = index;
                            updateFilterAndSort();
                        }
                    }
                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }

                ImGui::EndCombo();
            }

            ImGui::SameLine();
            if (ImGui::Button("OK", ImVec2(60.0f, 0.0f)))
            {
                selectFile();
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(60.0f, 0.0f)))
            {
                opened = false;
            }

            position = ImGui::GetWindowPos();
            size = ImGui::GetWindowSize();

            ImGui::EndPopup();
        }

        if (!opened)
        {
            save();
        }
    }
}