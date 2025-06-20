// Minimal ImGui OpenGL3 backend source stub
#include "imgui_impl_opengl3.h" // Should include imgui.h indirectly
// #include <glad/glad.h> // In a real build, this would be included for OpenGL functions
// For stub, avoid direct dependency if functions are not actually called or are stubbed in glad.c

// Minimal ImDrawData struct definition if not fully defined via imgui.h stub
// struct ImDrawData {}; // Already forward-declared in imgui_impl_opengl3.h, full def in imgui.h

IMGUI_IMPL_API bool ImGui_ImplOpenGL3_Init(const char* glsl_version) {
    (void)glsl_version;
    // In real backend, this sets up shaders, buffers, textures.
    // Stub returns true for success.
    // ImGuiIO& io = ImGui::GetIO();
    // io.BackendRendererName = "imgui_impl_opengl3_stub";
    return true;
}

IMGUI_IMPL_API void ImGui_ImplOpenGL3_Shutdown() {
    // In real backend, this cleans up OpenGL objects.
    // Stub is no-op.
}

IMGUI_IMPL_API void ImGui_ImplOpenGL3_NewFrame() {
    // In real backend, this might prepare GL state.
    // Stub is no-op.
}

IMGUI_IMPL_API void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data) {
    (void)draw_data;
    // In real backend, this is the core rendering function that uses OpenGL
    // to draw what ImGui has recorded in draw_data.
    // Stub is no-op.
}

// Optional stubs for other functions if they were declared and are needed
// bool ImGui_ImplOpenGL3_CreateFontsTexture() { return true; }
// void ImGui_ImplOpenGL3_DestroyFontsTexture() {}
// bool ImGui_ImplOpenGL3_CreateDeviceObjects() { return true; }
// void ImGui_ImplOpenGL3_DestroyDeviceObjects() {}
