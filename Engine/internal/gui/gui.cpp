#include "gui.h"
#include "debug/debug.h"
#include "tools/math2.h"
#include "tools/string.h"

#include "imgui.h"
#include "graphicscanvas.h"

static ImFont* imfont;
static char buffer[256];

void GUI::LoadFont(const char* file) {
	imfont = ImGui::GetIO().Fonts->AddFontFromFileTTF(file, 14);
}

void GUI::Begin(uint w, uint h) {
	if (imfont != nullptr) {
		ImGui::PushFont(imfont);
	}

	const int flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoTitleBar;
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(35 / 255.f, 38 / 255.f, 41 / 255.f, 1));
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
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();

	if (imfont != nullptr) {
		ImGui::PopFont();
	}

	ImGui::Render();
}

void GUI::Label(const char* title) {
	ImGui::Text(title);
}

bool GUI::Text(const char* title, std::string& v) {
	int len = Math::Min(IM_ARRAYSIZE(buffer) - 1, (int)v.length());
	strncpy(buffer, v.c_str(), len);
	buffer[len] = 0;

	if (ImGui::InputText(title, buffer, IM_ARRAYSIZE(buffer))) {
		v = buffer;
		return true;
	}

	return false;
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

bool GUI::Toggle(const char* title, bool* v) {
	return ImGui::Checkbox(title, v);
}

void GUI::Sameline() {
	ImGui::SameLine();
}

void GUI::Separator() {
	ImGui::Separator();
}

bool GUI::Popup(const char* title, int* selected, const char* items) {
	return ImGui::Combo(title, selected, items);
}

bool GUI::Slider(const char* title, float* v, float min, float max) {
	return ImGui::SliderFloat(title, v, min, max);
}

bool GUI::Float(const char* title, float* v) {
	return ImGui::DragFloat(title, v);
}

bool GUI::Float2(const char* title, float* v) {
	return ImGui::DragFloat2(title, v);
}

bool GUI::Float3(const char* title, float* v) {
	return ImGui::DragFloat3(title, v);
}

bool GUI::Float4(const char* title, float* v) {
	return ImGui::DragFloat4(title, v);
}

bool GUI::Color3(const char* title, float* v) {
	return ImGui::ColorEdit3(title, v);
}

bool GUI::Color4(const char* title, float* v) {
	return ImGui::ColorEdit4(title, v);
}

bool GUI::CollapsingHeader(const char* title) {
	return ImGui::CollapsingHeader(title, ImGuiTreeNodeFlags_DefaultOpen);
}
