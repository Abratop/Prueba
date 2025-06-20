// Minimal ImGui source stub for imgui_widgets.cpp
// This file in real ImGui contains implementations for various widgets.
#include "imgui.h"
#include <cstdio> // For NULL / vsnprintf if used by any complex widget stub

namespace ImGui {

    // Stubs for functions declared in imgui.h and typically implemented in imgui_widgets.cpp
    // We already provided some in imgui.cpp stub. Add others if they are separate.

    bool Button(const char* label, const ImVec2& size) {
        (void)label; (void)size;
        return false; // Simulate no button clicks
    }

    bool Checkbox(const char* label, bool* v) {
        (void)label; (void)v;
        return false; // Simulate no change
    }

    bool InputText(const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags, void* callback, void* user_data) {
        (void)label; (void)buf; (void)buf_size; (void)flags; (void)callback; (void)user_data;
        return false; // Simulate no text input change
    }

    bool Combo(const char* label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items) {
        (void)label; (void)current_item; (void)items; (void)items_count; (void)popup_max_height_in_items;
        return false; // Simulate no combo selection change
    }

    bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags) {
        (void)label; (void)v; (void)v_min; (void)v_max; (void)format; (void)flags;
        return false; // Simulate no slider change
    }

    bool ListBoxHeader(const char* label, const ImVec2& size) {
        (void)label; (void)size;
        return true; // Simulate listbox is open
    }

    void ListBoxFooter() {
        // no-op
    }

    bool Selectable(const char* label, bool selected, ImGuiSelectableFlags flags, const ImVec2& size) {
        (void)label; (void)selected; (void)flags; (void)size;
        return false; // Simulate no selection
    }

    // Add other widget function stubs if they are used by your GUI code
    // and not covered by the main imgui.cpp stub.

} // namespace ImGui
