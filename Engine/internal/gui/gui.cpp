#include "gui.h"
#include "debug/debug.h"
#include "tools/math2.h"
#include "tools/string.h"

#include "imgui.h"
#include "graphicscanvas.h"

static ImFont* imfont;
static char buffer[256];

#define FromGLMColor

void GUI::LoadFont(const char* file) {
	imfont = ImGui::GetIO().Fonts->AddFontFromFileTTF(file, 15, nullptr, ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());
}

void GUI::Begin(uint w, uint h, const Color& foregroundColor, const Color& backgroundColor) {
	if (imfont != nullptr) {
		ImGui::PushFont(imfont);
	}

	const int flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoTitleBar;
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1));
	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(foregroundColor.r, foregroundColor.g, foregroundColor.b, 1));

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

	ImGui::SetNextWindowPos(ImVec2(0, 0));

	ImVec2 size((float)w, (float)h);
	ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(size, size);

	ImGui::Begin("", nullptr, flags);
}

void GUI::End() {
	ImGui::End();

	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar(2);

	if (imfont != nullptr) {
		ImGui::PopFont();
	}

	ImGui::Render();
}

void GUI::LabelField(const char* text) {
	ImGui::Text(text);
}

void GUI::LabelField(const char* title, const char* text) {
	ImGui::LabelText(title, text);
}

bool GUI::TextField(const char* title, std::string& value) {
	int len = Math::Min(IM_ARRAYSIZE(buffer) - 1, (int)value.length());
	strncpy(buffer, value.c_str(), len);
	buffer[len] = 0;

	if (ImGui::InputText(title, buffer, IM_ARRAYSIZE(buffer))) {
		value = buffer;
		return true;
	}

	return false;
}

bool GUI::Button(const char* title) {
	return ImGui::Button(title);
}

void GUI::Image(const char* title, uint texture) {
	ImGui::Image((ImTextureID)(size_t)texture, ImVec2(50, 50));
	ImGui::SameLine();
	ImGui::Text(title);
}

bool GUI::ImageButton(const char* title, uint texture) {
	bool clicked = ImGui::ImageButton((ImTextureID)(size_t)texture, ImVec2(50, 50));
	ImGui::SameLine();
	ImGui::Text(title);
	return clicked;
}

bool GUI::Toggle(const char* title, bool& value) {
	return ImGui::Checkbox(title, &value);
}

void GUI::Sameline() {
	ImGui::SameLine();
}

void GUI::Separator() {
	ImGui::Separator();
}

void GUI::Indent(float value) {
	ImGui::Indent(value);
}

void GUI::Unindent(float value) {
	ImGui::Unindent(value);
}

bool GUI::Popup(const char* title, int* selected, const char* items) {
	return ImGui::Combo(title, selected, items);
}

bool GUI::MaskPopup(const char* title, int mask, const char* items_) {
	throw std::exception("not implemented");
	/*bool changed = false;
	int selectedMask = 0;
	if (ImGui::BeginCombo(title, "TODO")) {
		int value = 1;
		bool selected = false;
		if (ImGui::Selectable("None", &selected)) {
			mask = -1;
		}

		if (ImGui::Selectable("Everything", &selected)) {
			mask = 0;
		}

		for (int i = 0; *items_ != 0; ++i, mask >>= 1, value <<= 1) {
			for (; mask != 0; mask >>= 1, value <<= 1) {
				if ((mask & 1) != 0) {
					break;
				}
			}

			if (mask == 0) {
				Debug::LogError("invalid mask");
				return false;
			}

			selected = (value & (1 << i)) != 0;
			if (ImGui::Selectable(items_, &selected)) {
				if (selected) { selectedMask |= (1 << i); }
				else { selectedMask &= ~(1 << i); }

				changed = true;
			}

			items_ += strlen(items_) + 1;
		}

		ImGui::EndCombo();
	}*/

	//return changed;
}

bool GUI::Slider(const char* title, float& value, float min, float max) {
	return ImGui::SliderFloat(title, &value, min, max, "%.2f");
}

bool GUI::IntSlider(const char* title, int& value, int min, int max) {
	return ImGui::SliderInt(title, &value, min, max, "%.2f");
}

bool GUI::IntField(const char* title, int& value, int min, int max) {
	return ImGui::DragInt(title, &value, 1.f, min, max);
}

bool GUI::UIntField(const char* title, uint& value, uint min, uint max) {
	return ImGui::DragUInt(title, &value, 1.f, min, max);
}

bool GUI::FloatField(const char* title, float& value, float min, float max) {
	return ImGui::DragFloat(title, &value, 1.f, min, max);
}

bool GUI::Float2Field(const char* title, glm::vec2& value) {
	return ImGui::DragFloat2(title, (float*)&value);
}

bool GUI::Float3Field(const char* title, glm::vec3& value) {
	return ImGui::DragFloat3(title, (float*)&value);
}

bool GUI::Float4Field(const char* title, glm::vec4& value) {
	return ImGui::DragFloat4(title, (float*)&value);
}

bool GUI::RectField(const char* title, Rect& value) {
	return ImGui::DragFloat4(title, (float*)&value);
}

bool GUI::NormalizedRectField(const char* title, Rect& value) {
	return ImGui::DragFloat4(title, (float*)&value, 0.1f, 0, 1);
}

bool GUI::ColorField(const char* title, Color& value) {
	return ImGui::ColorEdit4(title, (float*)&value);
}

bool GUI::BeginMenu(const char * title) {
	return ImGui::BeginMenu(title);
}

bool GUI::MenuItem(const char * title, bool selected) {
	return ImGui::MenuItem(title, nullptr, selected);
}

void GUI::EndMenu() {
	ImGui::EndMenu();
}

bool GUI::CollapsingHeader(const char* title) {
	return ImGui::CollapsingHeader(title, ImGuiTreeNodeFlags_DefaultOpen);
}

void GUI::BeginScope(int id) {
	ImGui::PushID(id);
}

void GUI::BeginScope(const char* id) {
	ImGui::PushID(id);
}

void GUI::EndScope() {
	ImGui::PopID();
}
