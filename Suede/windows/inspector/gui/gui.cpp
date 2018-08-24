#include "gui.h"
#include "debug/debug.h"
#include "tools/math2.h"
#include "tools/string.h"

#include <qtimgui/QtImGui.h>

static ImFont* font;
static QWidget* view;
static char buffer[256];

void GUI::initialize(QWidget* widget) {
	view = widget;
	QtImGui::initialize(view);
	font = ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/fonts/tahoma.ttf", 14);
}

void GUI::destroy() {
	QtImGui::destroy();
}

void GUI::begin() {
	QtImGui::newFrame();
	ImGui::PushFont(font);

	const int flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoTitleBar;
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(35 / 255.f, 38 / 255.f, 41 / 255.f, 1));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::SetNextWindowPos(ImVec2(0, 0));

	ImVec2 size(view->width(), view->height());
	ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(size, size);

	ImGui::Begin("", nullptr, flags);
}

void GUI::end() {
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();

	ImGui::PopFont();

	ImGui::Render();
}

void GUI::text(const char* title) {
	ImGui::Text(title);
}

bool GUI::input(const char* title, std::string& v) {
	int len = Math::Min(IM_ARRAYSIZE(buffer) - 1, (int)v.length());
	strncpy(buffer, v.c_str(), len);
	buffer[len] = 0;

	if (ImGui::InputText(title, buffer, IM_ARRAYSIZE(buffer))) {
		v = buffer;
		return true;
	}

	return false;
}

void GUI::image(const char* title, uint texture) {
	ImGui::Image((ImTextureID)(size_t)texture, ImVec2(50, 50));
	ImGui::SameLine();
	ImGui::Text(title);
}

bool GUI::imageButton(const char* title, uint texture) {
	bool clicked = ImGui::ImageButton((ImTextureID)(size_t)texture, ImVec2(50, 50));
	ImGui::SameLine();
	ImGui::Text(title);
	return clicked;
}

bool GUI::checkbox(const char* title, bool* v) {
	return ImGui::Checkbox(title, v);
}

void GUI::sameline() {
	ImGui::SameLine();
}

void GUI::separator() {
	ImGui::Separator();
}

bool GUI::slider(const char* title, float* v, float min, float max) {
	return ImGui::SliderFloat(title, v, min, max);
}

bool GUI::single(const char* title, float* v) {
	return ImGui::DragFloat(title, v);
}

bool GUI::single2(const char* title, float* v) {
	return ImGui::DragFloat2(title, v);
}

bool GUI::single3(const char* title, float* v) {
	return ImGui::DragFloat3(title, v);
}

bool GUI::single4(const char* title, float* v) {
	return ImGui::DragFloat4(title, v);
}

bool GUI::color3(const char* title, float* v) {
	return ImGui::ColorEdit3(title, v);
}

bool GUI::color4(const char* title, float* v) {
	return ImGui::ColorEdit4(title, v);
}

bool GUI::collapsingHeader(const char* title) {
	return ImGui::CollapsingHeader(title, ImGuiTreeNodeFlags_DefaultOpen);
}

bool GUIPrivate::comboImpl(const char* title, int* selected, const char* items) {
	return ImGui::Combo(title, selected, items);
}
