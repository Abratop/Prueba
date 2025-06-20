#ifndef IMGUI_H
#define IMGUI_H

// Minimal ImGui stub

// Forward declarations for structs ImGui uses
struct ImGuiIO;
struct ImGuiStyle;
struct ImDrawData; // Forward declare ImDrawData

// Basic types
typedef unsigned int ImGuiID;
typedef int ImGuiWindowFlags;
typedef int ImGuiConfigFlags;
typedef int ImGuiInputTextFlags;
typedef int ImGuiSelectableFlags;
typedef int ImGuiTreeNodeFlags;
typedef int ImGuiComboFlags;
typedef int ImGuiSliderFlags;


// Enums (minimal subset)
enum ImGuiWindowFlags_ {
    ImGuiWindowFlags_None = 0,
    ImGuiWindowFlags_MenuBar = 1 << 0
    // ... other flags
};

enum ImGuiConfigFlags_ {
    ImGuiConfigFlags_None = 0,
    ImGuiConfigFlags_NavEnableKeyboard = 1 << 0
    // ... other flags
};


// Mock ImGuiIO structure
struct ImGuiIO {
    ImGuiConfigFlags ConfigFlags;
    // Add other fields if your code directly accesses them, e.g., DisplaySize
    // float DisplaySize[2];
    void** Fonts; // ImFontAtlas*
    ImGuiIO() : ConfigFlags(0), Fonts(nullptr) {}
};

// Mock ImGuiStyle structure
struct ImGuiStyle {
    float Alpha;
    // ... other style variables
    ImGuiStyle() : Alpha(1.0f) {}
};

// Basic ImVec2/ImVec4 for function signatures
struct ImVec2 { float x, y; ImVec2(float _x=0, float _y=0) : x(_x), y(_y) {} };
struct ImVec4 { float x, y, z, w; ImVec4(float _x=0, float _y=0, float _z=0, float _w=0) : x(_x),y(_y),z(_z),w(_w) {} };


namespace ImGui {
    // Context
    ImGuiIO& GetIO();
    ImGuiStyle& GetStyle();
    void CreateContext();
    void DestroyContext();

    // Main
    void NewFrame();
    void Render();
    ImDrawData* GetDrawData(); // Returns a pointer to ImDrawData

    // Window
    bool Begin(const char* name, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);
    void End();
    bool BeginMenuBar();
    void EndMenuBar();
    bool BeginMenu(const char* label, bool enabled = true);
    void EndMenu();
    bool MenuItem(const char* label, const char* shortcut = nullptr, bool selected = false, bool enabled = true);

    // Widgets
    void Text(const char* fmt, ...);
    bool Button(const char* label, const ImVec2& size = ImVec2(0,0));
    bool Checkbox(const char* label, bool* v);
    bool InputText(const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags = 0, void* callback = nullptr, void* user_data = nullptr);
    bool Combo(const char* label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items = -1);
    bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
    bool ListBoxHeader(const char* label, const ImVec2& size = ImVec2(0,0));
    void ListBoxFooter();
    bool Selectable(const char* label, bool selected = false, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0,0));


    // Styling
    void StyleColorsDark(ImGuiStyle* dst = nullptr);
    void StyleColorsLight(ImGuiStyle* dst = nullptr);
    void StyleColorsClassic(ImGuiStyle* dst = nullptr);

} // namespace ImGui

// Define IMGUI_CHECKVERSION to satisfy its usage
#define IMGUI_CHECKVERSION() ImGui::GetIO() // Simplified mock

// Define IM_ARRAYSIZE for convenience if used by UI code
#define IM_ARRAYSIZE(_ARR) ((int)(sizeof(_ARR)/sizeof(*_ARR)))


#endif // IMGUI_H
