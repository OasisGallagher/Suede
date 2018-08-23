#include "inspector.h"

#include "ui_suede.h"
#include "tagmanager.h"
#include "debug/debug.h"
#include "tools/math2.h"

#include <gl/glew.h>
#include <qtimgui/QtImGui.h>

#include "custom/meshinspector.h"
#include "custom/lightinspector.h"
#include "custom/camerainspector.h"
#include "custom/rendererinspector.h"
#include "custom/projectorinspector.h"

Inspector::Inspector(QWidget* parent) : QDockWidget(parent) {
	memset(namebuffer_, 0, sizeof(namebuffer_));
}

Inspector::~Inspector() {
	QtImGui::destroy();
}

void Inspector::init(Ui::Suede* ui) {
	WinBase::init(ui);

	connect(Hierarchy::instance(), SIGNAL(selectionChanged(const QList<Entity>&, const QList<Entity>&)),
		this, SLOT(onSelectionChanged(const QList<Entity>&, const QList<Entity>&)));
}

void Inspector::awake() {
	QtImGui::initialize(ui_->view);
	ui_->view->setFocusPolicy(Qt::StrongFocus);

	imguiFont_ = ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/fonts/tahoma.ttf", 14);
}

void Inspector::tick() {
	onGui();
}

void Inspector::onGui() {
	QGLContext* oldContext = (QGLContext*)QGLContext::currentContext();
	ui_->view->makeCurrent();

	QtImGui::newFrame();

	ImGui::PushFont(imguiFont_);

	const int windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoTitleBar;
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(35 / 255.f, 38 / 255.f, 41 / 255.f, 1));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(ui_->view->width(), ui_->view->height()), ImGuiCond_FirstUseEver);
	ImGui::Begin("", nullptr, windowFlags);

	if (target_) { drawGui(); }

	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();

	ImGui::PopFont();

	/*
	// 1. Show a simple window
	// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
	{
		static float f = 0.0f;
		ImGui::Text("Hello, world!");
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
		ImGui::ColorEdit3("clear color", (float*)&clear_color);
		if (ImGui::Button("Test Window")) show_test_window ^= 1;
		if (ImGui::Button("Another Window")) show_another_window ^= 1;
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	// 2. Show another simple window, this time using an explicit Begin/End pair
	if (show_another_window) {
		ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Another Window", &show_another_window);
		ImGui::Text("Hello");
		ImGui::End();
	}

	// 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if (show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow();
	}
	*/
	// Do render before ImGui UI is rendered
	//glViewport(0, 0, width(), height());
	//glClearColor(35.f / 255, 38.f / 255, 41.f / 255, 1);
	//glClear(GL_COLOR_BUFFER_BIT);

	ImGui::Render();

	ui_->view->swapBuffers();
	ui_->view->doneCurrent();

	if (oldContext != nullptr) {
		oldContext->makeCurrent();
	}
}

void Inspector::drawGui() {
	drawBasics();
	drawTransform();
	drawComponents();
}

void Inspector::drawBasics() {
	bool active = target_->GetActive();
	if (ImGui::Checkbox("Active", &active)) {
		target_->SetActiveSelf(active);
	}

	ImGui::SameLine();

	strncpy(namebuffer_, target_->GetName().c_str(), CountOf(namebuffer_) - 1);
	if (ImGui::InputText("Name", namebuffer_, CountOf(namebuffer_))) {
		target_->SetName(namebuffer_);
	}

	drawTags();
}

void Inspector::onSelectionChanged(const QList<Entity>& selected, const QList<Entity>& deselected) {
	// TODO: multi-selection.
	if (!selected.empty()) {
		target_ = selected.front();
	}
	else {
		target_ = nullptr;
	}
}

void Inspector::addInspector(CustomInspector* inspector) {
}

void Inspector::drawComponents() {
	switch (target_->GetType()) {
		case ObjectTypeCamera:
			drawCamera();
			break;
	}
	//switch (target_->GetType()) {
	//	case ObjectTypeCamera:
	//		addInspector(new CameraInspector(target_));
	//		break;
	//	case ObjectTypeProjector:
	//		addInspector(new ProjectorInspector(target_));
	//		break;
	//	case ObjectTypeSpotLight:
	//	case ObjectTypePointLight:
	//	case ObjectTypeDirectionalLight:
	//		addInspector(new LightInspector(target_));
	//		break;
	//}

	//if (target_->GetMesh()) {
	//	addInspector(new MeshInspector(target_->GetMesh()));
	//}

	//if (target_->GetRenderer()) {
	//	addInspector(new RendererInspector(target_->GetRenderer()));
	//}
}

void Inspector::drawCamera() {
	Camera camera = suede_dynamic_cast<Camera>(target_);

	int selected = -1;
	std::vector<char> buffer;
	for (int i = 0; i < ClearType::size(); ++i) {
		if (!buffer.empty()) { buffer.push_back(0); }
		if (camera->GetClearType() == ClearType::value(i)) {
			selected = i;
		}

		const char* str = ClearType::value(i).to_string();
		buffer.insert(buffer.end(), str, str + strlen(str));
	}

	buffer.push_back(0);
	buffer.push_back(0);

	if (ImGui::Combo("ClearType", &selected, buffer.data())) {
		camera->SetClearType(ClearType::value(selected));
	}

	float fieldOfView = camera->GetFieldOfView();
	if (ImGui::SliderFloat("FieldOfView", &fieldOfView, 1, 179)) {
		camera->SetFieldOfView(fieldOfView);
	}
}

void Inspector::drawTransform() {
	ImGui::Separator();
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
		glm::vec3 v3 = target_->GetTransform()->GetLocalPosition();
		if (ImGui::DragFloat3("P", (float*)&v3, 2)) {
			target_->GetTransform()->SetPosition(v3);
		}

		v3 = target_->GetTransform()->GetLocalEulerAngles();
		if (ImGui::DragFloat3("R", (float*)&v3, 2)) {
			target_->GetTransform()->SetLocalEulerAngles(v3);
		}

		v3 = target_->GetTransform()->GetLocalScale();
		if (ImGui::DragFloat3("S", (float*)&v3, 2)) {
			target_->GetTransform()->SetLocalScale(v3);
		}
	}
}

void Inspector::drawTags() {
	/*
	int tagIndex = -1;
	std::vector<std::string> tags;
	TagManager::instance()->GetAllTags(tags);
	std::vector<char> line;
	for (int i = 0; i < tags.size(); ++i) {
		const std::string& str = tags[i];
		if (tagIndex == -1 && str == target_->GetTag()) {
			tagIndex = i;
		}
		if (i != 0) {
			line.push_back(0);
		}

		line.insert(line.end(), str.begin(), str.end());
	}

	line.push_back(0);
	line.push_back(0);

	if (ImGui::Combo("Tag", &tagIndex, &line[0])) {
		target_->SetTag(tags[tagIndex]);
	}
	*/
}
