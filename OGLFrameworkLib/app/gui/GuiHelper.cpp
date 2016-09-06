/**
 * @file   GuiHelper.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.29
 *
 * @brief  Implementation of some GUI dialogs.
 */

#include "GuiHelper.h"
#include <imgui.h>

namespace cgu {

    std::tuple<GuiHelper::DialogReturn, std::string> GuiHelper::OpenFileDialog(const std::string& name, bool& showFileDialog)
    {
        auto result = std::make_tuple<DialogReturn, std::string>(DialogReturn::NO_RETURN, std::string(""));
        if (showFileDialog) ImGui::OpenPopup(name.c_str());
        if (ImGui::BeginPopupModal(name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            static std::string fileName;

            std::array<char, 1024> tmpFilename;
            auto lastPos = fileName.copy(tmpFilename.data(), 1023, 0);
            tmpFilename[lastPos] = '\0';
            if (ImGui::InputText("File Name", tmpFilename.data(), tmpFilename.size())) {
                fileName = tmpFilename.data();
            }

            if (ImGui::Button("OK")) {
                result = std::make_tuple<DialogReturn, std::string>(DialogReturn::OK, std::string(fileName));
                ImGui::CloseCurrentPopup();
                showFileDialog = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Close")) {
                result = std::make_tuple<DialogReturn, std::string>(DialogReturn::CANCEL, std::string(fileName));
                ImGui::CloseCurrentPopup();
                showFileDialog = false;
            }
            ImGui::EndPopup();
        }
        return result;
    }
}
