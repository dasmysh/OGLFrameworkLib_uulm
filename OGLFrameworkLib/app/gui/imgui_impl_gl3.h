/**
 * @file   imgui_impl_gl3.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.01.14
 *
 * @brief  Definitions of the ImGUI OpenGL 3 interface.
 */

#ifndef IMGUI_IMPL_GL3_H
#define IMGUI_IMPL_GL3_H

#include <imgui.h>
#include "main.h"

namespace imguiImpl {
    IMGUI_API bool ImGui_ImplGL3_Init(HWND hWnd);
    IMGUI_API void ImGui_ImplGL3_Shutdown();
    IMGUI_API void ImGui_ImplGL3_NewFrame(double current_time);
    void ImGui_ImplGL3_Resize(float width, float height);

    IMGUI_API void ImGui_ImplGL3_InvalidateDeviceObjects();
    IMGUI_API bool ImGui_ImplGL3_CreateDeviceObjects();

    IMGUI_API bool ImGui_ImplGL3_HandleMousePosition(float mouseX, float mouseY);
    IMGUI_API bool ImGui_ImplGL3_HandleMouse(unsigned int buttonAction, float mouseWheelDelta);
    IMGUI_API bool ImGui_ImplGL3_HandleKey(unsigned int vkCode, bool bKeyDown);
    IMGUI_API bool ImGui_ImplGL3_HandleChar(unsigned int c);
}


#endif // IMGUI_IMPL_GL3_H
