// Minimal ImGui source stub for imgui.cpp
#include "imgui.h"
#include <cstdio> // For NULL (if not included by imgui.h stub)

// Minimal static ImGuiIO instance
static ImGuiIO GImGuiIO;
// Minimal static ImGuiStyle instance
static ImGuiStyle GImGuiStyle;


namespace ImGui {
    ImGuiIO& GetIO() { return GImGuiIO; }
    ImGuiStyle& GetStyle() { return GImGuiStyle; }

    void CreateContext() {
        // Minimal context creation
        // In real ImGui, this allocates and initializes GImGui
    }
    void DestroyContext() {
        // Minimal context destruction
    }

    void NewFrame() { /* no-op for stub */ }
    void Render() { /* no-op for stub */ }

    // A very basic stub for GetDrawData. Real ImDrawData is complex.
    // We return a static null pointer or a dummy structure if absolutely necessary,
    // but for linking, just the signature might be enough if not dereferenced.
    static ImDrawData GImDrawData; // Dummy static instance
    ImDrawData* GetDrawData() {
        // Real ImGui populates this structure based on Render().
        // Stub just returns a pointer to a static dummy.
        return &GImDrawData;
    }

    bool Begin(const char* name, bool* p_open, ImGuiWindowFlags flags) {
        (void)name; (void)p_open; (void)flags;
        return true; // Always return true to allow drawing window content
    }
    void End() { /* no-op for stub */ }

    bool BeginMenuBar() { return true; } // Always allow menu bar content
    void EndMenuBar() { /* no-op for stub */ }
    bool BeginMenu(const char* label, bool enabled) { (void)label; (void)enabled; return true; } // Always allow menu content
    void EndMenu() { /* no-op for stub */ }
    bool MenuItem(const char* label, const char* shortcut, bool selected, bool enabled) {
        (void)label; (void)shortcut; (void)selected; (void)enabled;
        return false; // Simulate no menu item clicks
    }

    // Basic stub for Text to avoid link errors
    void Text(const char* fmt, ...) { (void)fmt; /* no-op */ }

    // Other functions defined in imgui.h would need minimal stubs here if called
    // For example:
    void StyleColorsDark(ImGuiStyle* dst) { (void)dst; /* no-op */ }
    void StyleColorsLight(ImGuiStyle* dst) { (void)dst; /* no-op */ }
    void StyleColorsClassic(ImGuiStyle* dst) { (void)dst; /* no-op */ }

} // namespace ImGui
