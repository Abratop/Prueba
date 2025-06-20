#ifndef IMGUI_IMPL_GLFW_H
#define IMGUI_IMPL_GLFW_H

// Minimal ImGui GLFW backend header stub

// Forward declare GLFWwindow
struct GLFWwindow; // Or #include <GLFW/glfw3.h> if its path is known to the stub
                  // For a self-contained stub, forward declaration is safer.

IMGUI_IMPL_API bool ImGui_ImplGlfw_InitForOpenGL(GLFWwindow* window, bool install_callbacks);
IMGUI_IMPL_API void ImGui_ImplGlfw_Shutdown();
IMGUI_IMPL_API void ImGui_ImplGlfw_NewFrame();

// These are often called by the user's GLFW callbacks if `install_callbacks` is true.
// If `install_callbacks` is false, the user needs to call them.
// For stub purposes, their presence in the API is what matters.
IMGUI_IMPL_API void ImGui_ImplGlfw_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
IMGUI_IMPL_API void ImGui_ImplGlfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
IMGUI_IMPL_API void ImGui_ImplGlfw_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
IMGUI_IMPL_API void ImGui_ImplGlfw_CharCallback(GLFWwindow* window, unsigned int c);


// Helper macro (copied from real ImGui backends)
#ifndef IMGUI_IMPL_API
#define IMGUI_IMPL_API
#endif

#endif // IMGUI_IMPL_GLFW_H
