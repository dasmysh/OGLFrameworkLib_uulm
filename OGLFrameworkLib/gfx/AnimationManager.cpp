/**
 * @file   AnimationManager.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.25
 *
 * @brief  Implementation of the animation manager.
 */

#include "AnimationManager.h"
#include <imgui.h>
#include "WaypointAnimation.h"
#include <core/AnimationManagerSerializationHelper.h>

namespace cgu {

    AnimationManager::AnimationManager(const std::string& dir) :
        directory_(dir)
    {
    }

    AnimationManager::~AnimationManager() = default;

    unsigned AnimationManager::AddAnimation(const std::string& name)
    {
        animations_.emplace_back(WaypointAnimation());
        auto id = static_cast<unsigned>(animations_.size() - 1);
        animationsByName_.insert(std::make_pair(name, std::make_pair(id, &animations_.back())));
        return id;
    }

    void AnimationManager::ShowAnimationMenu(const std::string& name)
    {
        static auto showLoadAnimationPopup = false;
        static auto showSaveAnimationPopup = false;
        static auto showSelectEditAnimationPopup = false;
        if (ImGui::BeginMenu(name.c_str())) {
            ImGui::MenuItem("Load Animation", nullptr, &showLoadAnimationPopup);
            ImGui::MenuItem("Save Animation", nullptr, &showSaveAnimationPopup);
            ImGui::MenuItem("Select Edit Animation", nullptr, &showSelectEditAnimationPopup);
            ImGui::EndMenu();
        }

        if (showLoadAnimationPopup) ImGui::OpenPopup("Load Animation");
        if (ImGui::BeginPopupModal("Load Animation", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            static auto waypointSet = 0;
            static std::string wpFileName;

            std::array<char, 1024> tmpFilename;
            auto lastPos = wpFileName.copy(tmpFilename.data(), 1023, 0);
            tmpFilename[lastPos] = '\0';
            if (ImGui::InputText("File Name", tmpFilename.data(), tmpFilename.size())) {
                wpFileName = tmpFilename.data();
            }

            for (const auto& set : animationsByName_) ImGui::RadioButton(set.first.c_str(), &waypointSet, set.second.first);
            if (ImGui::Button("Load")) {
                LoadAnimation(wpFileName, waypointSet);
                ImGui::CloseCurrentPopup();
                showLoadAnimationPopup = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
                showLoadAnimationPopup = false;
            }
            ImGui::EndPopup();
        }

        if (showSaveAnimationPopup) ImGui::OpenPopup("Save Animation");
        if (ImGui::BeginPopupModal("Save Animation", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            static auto waypointSet = 0;
            static std::string wpFileName;

            std::array<char, 1024> tmpFilename;
            auto lastPos = wpFileName.copy(tmpFilename.data(), 1023, 0);
            tmpFilename[lastPos] = '\0';
            if (ImGui::InputText("File Name", tmpFilename.data(), tmpFilename.size())) {
                wpFileName = tmpFilename.data();
            }

            for (const auto& set : animationsByName_) ImGui::RadioButton(set.first.c_str(), &waypointSet, set.second.first);
            if (ImGui::Button("Save")) {
                SaveAnimation(wpFileName, waypointSet);
                ImGui::CloseCurrentPopup();
                showSaveAnimationPopup = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
                showSaveAnimationPopup = false;
            }
            ImGui::EndPopup();
        }



        if (showSelectEditAnimationPopup) ImGui::OpenPopup("Select Edit Animation");
        if (ImGui::BeginPopupModal("Select Edit Animation", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            auto waypointSet = static_cast<int>(currentAnimation_);

            for (const auto& set : animationsByName_) ImGui::RadioButton(set.first.c_str(), &waypointSet, set.second.first);
            if (ImGui::Button("Edit")) {
                currentAnimation_ = static_cast<unsigned>(waypointSet);
                ImGui::CloseCurrentPopup();
                showSelectEditAnimationPopup = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
                showSelectEditAnimationPopup = false;
            }
            ImGui::EndPopup();
        }
    }

    void AnimationManager::LoadAnimation(const std::string& filename, int set)
    {
        std::ifstream wpFile(directory_ + "/" + filename, std::ios::in);
        if (wpFile.is_open()) AnimationManagerSerializationHelper::LoadAnimation(wpFile, animations_[set]);
    }

    void AnimationManager::SaveAnimation(const std::string& filename, int set)
    {
        std::ofstream ofs(directory_ + "/" + filename, std::ios::out);
        AnimationManagerSerializationHelper::SaveAnimation(ofs, animations_[set]);
    }

    void AnimationManager::LoadAll(const std::string& filename)
    {
        animationsByName_.clear();
        animations_.clear();
        std::vector<std::string> animationNames;
        std::ifstream wpFile(directory_ + "/" + filename, std::ios::in);
        AnimationManagerSerializationHelper::LoadAnimations(wpFile, animations_, animationNames);

        for (unsigned int i = 0; i < animations_.size(); ++i) {
            animationsByName_.insert(std::make_pair(animationNames[i], std::make_pair(i, &animations_[i])));
        }
    }

    void AnimationManager::SaveAll(const std::string& filename)
    {
        std::vector<std::string> animationNames(animations_.size());
        for (const auto& set : animationsByName_) animationNames[set.second.first] = set.first;

        std::ofstream ofs(directory_ + "/" + filename, std::ios::out);
        AnimationManagerSerializationHelper::SaveAnimations(ofs, animations_, animationNames);
    }
}
