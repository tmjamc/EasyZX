#include <string>
#include <vector>

#include "ui.h"
#include "widgets_window.h"
#include "ini.h"
#include "frame_time.h"
#include "model_selection.h"
#include "giga_screen.h"
#include "disk_manager.h"
#include "tape_manager.h"

namespace widgets_window
{
    namespace
    {
        enum Type { MODEL, GIGA, DISK, TAPE, FRAME, CONTROL, GROUP };

        struct Widget
        {
            Type type;
            std::string name;
            bool collapsed;
            std::vector<Widget> children;
        };
        
        struct Window
        {
            float x;
            float y;
            float width;
            float height;
            std::string title;
            std::vector<Widget> widgets;
        };

        std::vector<Window> windows;

        void addWidgets(mINI::INIMap<std::string> section, int &index, std::vector<Widget>* widgets)
        {
            std::string value;

            while ((value = section.get(std::format("w{:02d}", index++))) != "")
            {
                if (value == "model_selection")
                {
                    widgets->emplace_back(Type::MODEL);
                    continue;
                }
                if (value == "giga_screen")
                {
                    widgets->emplace_back(Type::GIGA);
                    continue;
                }
                if (value == "disk_manager")
                {
                    widgets->emplace_back(Type::DISK);
                    continue;
                }
                if (value == "tape_manager")
                {
                    widgets->emplace_back(Type::TAPE);
                    continue;
                }
                if (value == "frame_time")
                {
                    widgets->emplace_back(Type::FRAME);
                    continue;
                }
                if (value == "control")
                {
                    widgets->emplace_back(Type::CONTROL);
                    continue;
                }
                if (value == "end_group")
                {
                    return;
                }
                if (value.starts_with("start_group:"))
                {
                    addWidgets(section, index, &(widgets->emplace_back(Type::GROUP, value.substr(12)).children));
                }
            }
        }

        void renderWidgets(std::vector<Widget>* widgets)
        {
            for (Widget &widget : *widgets)
            {
                switch (widget.type)
                {
                case Type::MODEL:
                    widgets::renderModelSelection();
                    break;
                case Type::GIGA:
                    widgets::renderGigaScreen();
                    break;
                case Type::DISK:
                    widgets::renderDiskManager();
                    break;
                case Type::TAPE:
                    widgets::renderTapeManager();
                    break;
                case Type::FRAME:
                    widgets::renderFrameTime();
                    break;
                // case Type::CONTROL:
                //     widgets::renderControl();
                //     break;

                case Type::GROUP:
                    if (ui::CollapsingHeader(widget.name.c_str(), widget.collapsed))
                    {
                        renderWidgets(&(widget.children));
                    }
                    break;
                }
            }
        }
    }

    void init()
    {
        mINI::INIFile file("./window_test_1.ini");

        mINI::INIStructure ini;

        if (!file.read(ini))
        {
            return;
        }

        const auto &sectionWindow = ini["window"];
        std::vector<Widget>* widgets = &(windows.emplace_back(std::stof(sectionWindow.get("x")), std::stof(sectionWindow.get("y")), std::stof(sectionWindow.get("width")), std::stof(sectionWindow.get("height")), sectionWindow.get("title")).widgets);

        const auto &sectionWidgets = ini["widgets"];
        int index = 0;
        addWidgets(sectionWidgets, index, widgets);
    }

    void render()
    {
        for (Window &window : windows)
        {
            ImGui::SetNextWindowSize(ImVec2(window.x, window.y), ImGuiCond_Once);
            ImGui::SetNextWindowSize(ImVec2(window.width, window.height), ImGuiCond_Once);
            if (ui::Begin(window.title.c_str(), ImGuiWindowFlags_NoCollapse))
            {
                renderWidgets(&(window.widgets));
            }
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 10.0f);
            ImGui::Dummy(ImVec2(0.0f, 0.0f));
            ImGui::End();
        }
    }
}