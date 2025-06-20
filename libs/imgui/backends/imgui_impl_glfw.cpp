// Minimal ImGui GLFW backend source stub
#include "imgui_impl_glfw.h" // Should include imgui.h indirectly
// #include <GLFW/glfw3.h> // In a real build, this would be included. For stub, avoid direct dependency if possible.

IMGUI_IMPL_API bool ImGui_ImplGlfw_InitForOpenGL(GLFWwindow* window, bool install_callbacks) {
    (void)window; (void)install_callbacks;
    // In real backend, this sets up callbacks, clipboard, mouse cursors etc.
    // Stub returns true to indicate success.
    return true;
}

IMGUI_IMPL_API void ImGui_ImplGlfw_Shutdown() {
    // In real backend, this clears callbacks and allocated data.
    // Stub is no-op.
}

IMGUI_IMPL_API void ImGui_ImplGlfw_NewFrame() {
    // In real backend, this reads mouse/keyboard/gamepad state, window size, etc.
    // Stub is no-op.
    // ImGuiIO& io = ImGui::GetIO();
    // io.DisplaySize = ImVec2(1280, 720); // Example: set a dummy display size if needed
    // io.DeltaTime = 1.0f / 60.0f;      // Example: set a dummy delta time
}

// Stubs for callback functions (these would normally update ImGui's IO state)
IMGUI_IMPL_API void ImGui_ImplGlfw_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    (void)window; (void)button; (void)action; (void)mods;
}
IMGUI_IMPL_API void ImGui_ImplGlfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window; (void)xoffset; (void)yoffset;
}
IMGUI_IMPL_API void ImGui_ImplGlfw_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)window; (void)key; (void)scancode; (void)action; (void)mods;
}
IMGUI_IMPL_API void ImGui_ImplGlfw_CharCallback(GLFWwindow* window, unsigned int c) {
    (void)window; (void)c;
}
