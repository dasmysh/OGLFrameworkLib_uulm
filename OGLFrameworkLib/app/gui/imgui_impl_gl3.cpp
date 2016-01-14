/**
 * @file   imgui_impl_gl3.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.01.14
 *
 * @brief  Implementation of the ImGUI OpenGL 3 interface. Mostly copied from ImGUI GLFW example.
 */

#include "imgui_impl_gl3.h"

namespace imguiImpl {
    static double       g_Time = 0.0f;
    static bool         g_MousePressed[5] = { false, false, false, false, false };
    static float        g_MouseWheel = 0.0f;
    static float        g_mouseX = 0.0f, g_mouseY = 0.0f;
    static GLuint       g_FontTexture = 0;
    static int          g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
    static int          g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;
    static int          g_AttribLocationPosition = 0, g_AttribLocationUV = 0, g_AttribLocationColor = 0;
    static unsigned int g_VboHandle = 0, g_VaoHandle = 0, g_ElementsHandle = 0;

    // This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
    // If text or lines are blurry when integrating ImGui in your engine:
    // - in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
    void ImGui_ImplGL3_RenderDrawLists(ImDrawData* draw_data)
    {
        // Backup GL state
        GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
        GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
        GLint last_element_array_buffer; glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
        GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
        GLint last_blend_src; glGetIntegerv(GL_BLEND_SRC, &last_blend_src);
        GLint last_blend_dst; glGetIntegerv(GL_BLEND_DST, &last_blend_dst);
        GLint last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, &last_blend_equation_rgb);
        GLint last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &last_blend_equation_alpha);
        GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
        auto last_enable_blend = glIsEnabled(GL_BLEND);
        auto last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
        auto last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
        auto last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

        // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_SCISSOR_TEST);
        glActiveTexture(GL_TEXTURE0);

        // Handle cases of screen coordinates != from framebuffer coordinates (e.g. retina displays)
        auto& io = ImGui::GetIO();
        auto fb_width = static_cast<int>(io.DisplaySize.x * io.DisplayFramebufferScale.x);
        auto fb_height = static_cast<int>(io.DisplaySize.y * io.DisplayFramebufferScale.y);
        draw_data->ScaleClipRects(io.DisplayFramebufferScale);

        // Setup view-port, orthographic projection matrix
        glViewport(0, 0, static_cast<GLsizei>(fb_width), static_cast<GLsizei>(fb_height));
        const float ortho_projection[4][4] =
        {
            { 2.0f / io.DisplaySize.x, 0.0f, 0.0f, 0.0f },
            { 0.0f, 2.0f / -io.DisplaySize.y, 0.0f, 0.0f },
            { 0.0f, 0.0f, -1.0f, 0.0f },
            { -1.0f, 1.0f, 0.0f, 1.0f },
        };
        glUseProgram(g_ShaderHandle);
        glUniform1i(g_AttribLocationTex, 0);
        glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
        glBindVertexArray(g_VaoHandle);

        for (auto n = 0; n < draw_data->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = draw_data->CmdLists[n];
            const ImDrawIdx* idx_buffer_offset = nullptr;

            glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
            glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(cmd_list->VtxBuffer.size()) * sizeof(ImDrawVert), reinterpret_cast<const GLvoid*>(&cmd_list->VtxBuffer.front()), GL_STREAM_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(cmd_list->IdxBuffer.size()) * sizeof(ImDrawIdx), reinterpret_cast<const GLvoid*>(&cmd_list->IdxBuffer.front()), GL_STREAM_DRAW);

            for (auto pcmd = cmd_list->CmdBuffer.begin(); pcmd != cmd_list->CmdBuffer.end(); pcmd++)
            {
                if (pcmd->UserCallback)
                {
                    pcmd->UserCallback(cmd_list, pcmd);
                } else
                {
                    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(reinterpret_cast<intptr_t>(pcmd->TextureId)));
                    glScissor(static_cast<int>(pcmd->ClipRect.x), static_cast<int>(fb_height - pcmd->ClipRect.w), static_cast<int>(pcmd->ClipRect.z - pcmd->ClipRect.x), static_cast<int>(pcmd->ClipRect.w - pcmd->ClipRect.y));
                    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(pcmd->ElemCount), sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
                }
                idx_buffer_offset += pcmd->ElemCount;
            }
        }

        // Restore modified GL state
        glUseProgram(last_program);
        glBindTexture(GL_TEXTURE_2D, last_texture);
        glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
        glBindVertexArray(last_vertex_array);
        glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
        glBlendFunc(last_blend_src, last_blend_dst);
        if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
        if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
        if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
        glViewport(last_viewport[0], last_viewport[1], static_cast<GLsizei>(last_viewport[2]), static_cast<GLsizei>(last_viewport[3]));
    }

    bool ImGui_ImplGL3_CreateFontsTexture()
    {
        // Build texture atlas
        auto& io = ImGui::GetIO();
        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits for OpenGL3 demo because it is more likely to be compatible with user's existing shader.

        // Upload texture to graphics system
        GLint last_texture;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
        glGenTextures(1, &g_FontTexture);
        glBindTexture(GL_TEXTURE_2D, g_FontTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

        // Store our identifier
        io.Fonts->TexID = reinterpret_cast<void *>(static_cast<intptr_t>(g_FontTexture));

        // Restore state
        glBindTexture(GL_TEXTURE_2D, last_texture);

        return true;
    }

    bool ImGui_ImplGL3_CreateDeviceObjects()
    {
        // Backup GL state
        GLint last_texture, last_array_buffer, last_vertex_array;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

        auto vertex_shader =
            "#version 330\n"
            "uniform mat4 ProjMtx;\n"
            "in vec2 Position;\n"
            "in vec2 UV;\n"
            "in vec4 Color;\n"
            "out vec2 Frag_UV;\n"
            "out vec4 Frag_Color;\n"
            "void main()\n"
            "{\n"
            "	Frag_UV = UV;\n"
            "	Frag_Color = Color;\n"
            "	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
            "}\n";

        auto fragment_shader =
            "#version 330\n"
            "uniform sampler2D Texture;\n"
            "in vec2 Frag_UV;\n"
            "in vec4 Frag_Color;\n"
            "out vec4 Out_Color;\n"
            "void main()\n"
            "{\n"
            "	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
            "}\n";

        g_ShaderHandle = glCreateProgram();
        g_VertHandle = glCreateShader(GL_VERTEX_SHADER);
        g_FragHandle = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(g_VertHandle, 1, &vertex_shader, nullptr);
        glShaderSource(g_FragHandle, 1, &fragment_shader, nullptr);
        glCompileShader(g_VertHandle);
        glCompileShader(g_FragHandle);
        glAttachShader(g_ShaderHandle, g_VertHandle);
        glAttachShader(g_ShaderHandle, g_FragHandle);
        glLinkProgram(g_ShaderHandle);

        g_AttribLocationTex = glGetUniformLocation(g_ShaderHandle, "Texture");
        g_AttribLocationProjMtx = glGetUniformLocation(g_ShaderHandle, "ProjMtx");
        g_AttribLocationPosition = glGetAttribLocation(g_ShaderHandle, "Position");
        g_AttribLocationUV = glGetAttribLocation(g_ShaderHandle, "UV");
        g_AttribLocationColor = glGetAttribLocation(g_ShaderHandle, "Color");

        glGenBuffers(1, &g_VboHandle);
        glGenBuffers(1, &g_ElementsHandle);

        glGenVertexArrays(1, &g_VaoHandle);
        glBindVertexArray(g_VaoHandle);
        glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
        glEnableVertexAttribArray(g_AttribLocationPosition);
        glEnableVertexAttribArray(g_AttribLocationUV);
        glEnableVertexAttribArray(g_AttribLocationColor);

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
        glVertexAttribPointer(g_AttribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), reinterpret_cast<GLvoid*>(offsetof(ImDrawVert, pos)));
        glVertexAttribPointer(g_AttribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), reinterpret_cast<GLvoid*>(offsetof(ImDrawVert, uv)));
        glVertexAttribPointer(g_AttribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), reinterpret_cast<GLvoid*>(offsetof(ImDrawVert, col)));
#undef OFFSETOF

        ImGui_ImplGL3_CreateFontsTexture();

        // Restore modified GL state
        glBindTexture(GL_TEXTURE_2D, last_texture);
        glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
        glBindVertexArray(last_vertex_array);

        return true;
    }

    bool ImGui_ImplGL3_HandleMousePosition(float mouseX, float mouseY)
    {
        g_mouseX = mouseX;
        g_mouseY = mouseY;

        auto& io = ImGui::GetIO();
        return io.WantCaptureMouse;
    }

    bool ImGui_ImplGL3_HandleMouse(unsigned buttonAction, float mouseWheelDelta)
    {
        if (buttonAction & RI_MOUSE_LEFT_BUTTON_DOWN) g_MousePressed[0] = true;
        if (buttonAction & RI_MOUSE_RIGHT_BUTTON_DOWN) g_MousePressed[1] = true;
        if (buttonAction & RI_MOUSE_MIDDLE_BUTTON_DOWN) g_MousePressed[2] = true;
        if (buttonAction & RI_MOUSE_BUTTON_4_DOWN) g_MousePressed[3] = true;
        if (buttonAction & RI_MOUSE_BUTTON_5_DOWN) g_MousePressed[4] = true;
        g_MouseWheel += static_cast<float>(mouseWheelDelta); // Use fractional mouse wheel, 1.0 unit 5 lines.

        auto& io = ImGui::GetIO();
        return io.WantCaptureMouse;
    }

    bool ImGui_ImplGL3_HandleKey(unsigned int vkCode, bool bKeyDown)
    {
        auto& io = ImGui::GetIO();
        if (bKeyDown) io.KeysDown[vkCode] = true;
        else io.KeysDown[vkCode] = false;

        io.KeyCtrl = io.KeysDown[VK_LCONTROL] || io.KeysDown[VK_RCONTROL];
        io.KeyShift = io.KeysDown[VK_LSHIFT] || io.KeysDown[VK_RSHIFT];
        io.KeyAlt = io.KeysDown[VK_LMENU] || io.KeysDown[VK_RMENU];
        return io.WantCaptureKeyboard;
    }

    bool ImGui_ImplGL3_HandleChar(unsigned c)
    {
        auto& io = ImGui::GetIO();
        if (c > 0 && c < 0x10000)
            io.AddInputCharacter(static_cast<unsigned short>(c));

        return io.WantCaptureKeyboard;
    }

    void    ImGui_ImplGL3_InvalidateDeviceObjects()
    {
        if (g_VaoHandle) glDeleteVertexArrays(1, &g_VaoHandle);
        if (g_VboHandle) glDeleteBuffers(1, &g_VboHandle);
        if (g_ElementsHandle) glDeleteBuffers(1, &g_ElementsHandle);
        g_VaoHandle = g_VboHandle = g_ElementsHandle = 0;

        glDetachShader(g_ShaderHandle, g_VertHandle);
        glDeleteShader(g_VertHandle);
        g_VertHandle = 0;

        glDetachShader(g_ShaderHandle, g_FragHandle);
        glDeleteShader(g_FragHandle);
        g_FragHandle = 0;

        glDeleteProgram(g_ShaderHandle);
        g_ShaderHandle = 0;

        if (g_FontTexture)
        {
            glDeleteTextures(1, &g_FontTexture);
            ImGui::GetIO().Fonts->TexID = nullptr;
            g_FontTexture = 0;
        }
    }

    bool    ImGui_ImplGL3_Init(HWND hWnd)
    {
        auto& io = ImGui::GetIO();
        io.KeyMap[ImGuiKey_Tab] = VK_TAB;                         // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
        io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
        io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
        io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
        io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
        io.KeyMap[ImGuiKey_Home] = VK_HOME;
        io.KeyMap[ImGuiKey_End] = VK_END;
        io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
        io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
        io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
        io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
        io.KeyMap[ImGuiKey_A] = 'A';
        io.KeyMap[ImGuiKey_C] = 'C';
        io.KeyMap[ImGuiKey_V] = 'V';
        io.KeyMap[ImGuiKey_X] = 'X';
        io.KeyMap[ImGuiKey_Y] = 'Y';
        io.KeyMap[ImGuiKey_Z] = 'Z';

        io.RenderDrawListsFn = ImGui_ImplGL3_RenderDrawLists;       // Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
        io.ImeWindowHandle = hWnd;

        return true;
    }

    void ImGui_ImplGL3_Shutdown()
    {
        ImGui_ImplGL3_InvalidateDeviceObjects();
        ImGui::Shutdown();
    }

    void ImGui_ImplGL3_Resize(float width, float height)
    {
        auto& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(width, height);
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    }

    void ImGui_ImplGL3_NewFrame(double current_time)
    {
        if (!g_FontTexture)
            ImGui_ImplGL3_CreateDeviceObjects();

        auto& io = ImGui::GetIO();

        // Setup time step
        io.DeltaTime = g_Time > 0.0 ? static_cast<float>(current_time - g_Time) : static_cast<float>(1.0f / 60.0f);
        g_Time = current_time;

        // Setup inputs
        // (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
        //if (glfwGetWindowAttrib(g_Window, GLFW_FOCUSED))
        //{
            io.MousePos = ImVec2(g_mouseX, g_mouseY);   // Mouse position in screen coordinates (set to -1,-1 if no mouse / on another screen, etc.)
        /*} else
        {
            io.MousePos = ImVec2(-1, -1);
        }*/

        for (auto i = 0; i < 5; i++)
        {
            io.MouseDown[i] = g_MousePressed[i];    // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
            g_MousePressed[i] = false;
        }

        io.MouseWheel = g_MouseWheel;
        g_MouseWheel = 0.0f;

        // Hide OS mouse cursor if ImGui is drawing it
        ShowCursor(io.MouseDrawCursor ? FALSE : TRUE);

        // Start the frame
        ImGui::NewFrame();
    }
}
