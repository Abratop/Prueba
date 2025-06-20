// Minimal ImGui source stub for imgui_draw.cpp
// This file in real ImGui contains all the low-level drawing commands (vertices, indices, etc.)
#include "imgui.h" // May need internal types if used

// If imgui_internal.h types are needed by functions that would normally be in this file,
// those types would also need to be stubbed or the functions simplified.
// For a basic linkable stub, this can often be empty if no symbols are explicitly called
// from other cpp files that are part of our stubs.
// However, ImGui::Render() calls functions that are typically in imgui_draw.cpp.
// Our ImGui::Render() stub is a no-op, so this might be fine.

// Example of a function that might be in imgui_draw.cpp, if our stubs needed it:
// namespace ImGui {
//     void AddText(ImVec2 pos, ImU32 col, const char* text_begin, const char* text_end) {
//         (void)pos; (void)col; (void)text_begin; (void)text_end;
//     }
// }
