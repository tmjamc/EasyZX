#include "imgui.h"
#include "realtime_monitor.h"
#include "main.h"
#include "settings.h"

namespace realtime_monitor
{
    namespace
    {
        const main::Model* selectedModel = nullptr;
    }

    bool opened = true;

    void render()
    {
        if (selectedModel == nullptr)
        {
            selectedModel = main::currentModel;
        }

        ImGui::SetNextWindowSize(ImVec2(300, 80), ImGuiCond_Once);
        if (ImGui::Begin("Realtime monitor", &opened, ImGuiWindowFlags_NoCollapse))
        {
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Model:");

            ImGui::SameLine();
            if (ImGui::BeginCombo("##model", selectedModel->name))
            {
                for (const main::Model &model : main::models)
                {
                    const bool is_selected = (model.id == selectedModel->id);
                    if (ImGui::Selectable(model.name, is_selected))
                    {
                        if (selectedModel->id != model.id)
                        {
                            selectedModel = &model;
                            settings::current.modelId = selectedModel->id;
                            settings::save();
                            main::resetRequested = true;
                        }
                    }
                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }

                ImGui::EndCombo();
            }
        }
        ImGui::End();
    }
}