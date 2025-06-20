#ifndef IMGUI_IMPL_OPENGL3_H
#define IMGUI_IMPL_OPENGL3_H

// Minimal ImGui OpenGL3 backend header stub

// Forward declare ImDrawData (already in imgui.h stub, but good for self-containment)
struct ImDrawData;

IMGUI_IMPL_API bool ImGui_ImplOpenGL3_Init(const char* glsl_version = nullptr);
IMGUI_IMPL_API void ImGui_ImplOpenGL3_Shutdown();
IMGUI_IMPL_API void ImGui_ImplOpenGL3_NewFrame();
IMGUI_IMPL_API void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data);

// Optional: other functions if needed by your specific setup
// IMGUI_IMPL_API bool ImGui_ImplOpenGL3_CreateFontsTexture();
// IMGUI_IMPL_API void ImGui_ImplOpenGL3_DestroyFontsTexture();
// IMGUI_IMPL_API bool ImGui_ImplOpenGL3_CreateDeviceObjects();
// IMGUI_IMPL_API void ImGui_ImplOpenGL3_DestroyDeviceObjects();

// Helper macro (copied from real ImGui backends)
#ifndef IMGUI_IMPL_API
#define IMGUI_IMPL_API
#endif

#endif // IMGUI_IMPL_OPENGL3_H
