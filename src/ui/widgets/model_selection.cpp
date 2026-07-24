#include "imgui.h"
#include "main.h"
#include "model_selection.h"
#include "settings.h"

namespace widgets
{
    namespace
    {
        const main::Model* selectedModel = nullptr;
    }

    void renderModelSelection()
    {
        if (selectedModel == nullptr)
        {
            selectedModel = main::currentModel;
        }

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
}