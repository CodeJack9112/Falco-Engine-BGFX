#pragma once

#include <imgui.h>
#include <utility>
#include <vector>
#include <string>
#include <functional>

namespace ImGui
{
	IMGUI_API bool ListBox2(const char* label, int* current_item, std::vector<std::pair<std::string, std::string>> const items, int height_items, std::function<void(int i)> btnCallback = nullptr);
	IMGUI_API bool TreeNodeEx2(const char* str_id, ImGuiTreeNodeFlags flags, const char* fmt, ...);
	IMGUI_API bool TreeNodeExV2(const char* str_id, ImGuiTreeNodeFlags flags, const char* fmt, va_list args);
	IMGUI_API bool TreeNodeBehavior2(ImGuiID id, ImGuiTreeNodeFlags flags, const char* label, const char* label_end);
	IMGUI_API bool InvisibleButton2(const char* str_id, const ImVec2& size);
	IMGUI_API bool ImageButtonWithID(ImTextureID user_texture_id, const ImVec2& size, ImGuiID ID, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1));
	IMGUI_API void AddTextVertical(ImDrawList* DrawList, const char* text, ImVec2 pos, ImU32 text_color);
	IMGUI_API float CalcTextSizeVertical(const char* text);
	IMGUI_API bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f, const char* custom_id = nullptr);
	IMGUI_API bool ImageButtonWithText(ImTextureID texId, const char* label, const ImVec2& imageSize = ImVec2(0, 0), const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec2& text_size = ImVec2(-1, -1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const char* custom_id = nullptr);
}
