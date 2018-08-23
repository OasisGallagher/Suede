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

ImFont* imguiFont = nullptr;
Inspector::Inspector(QWidget* parent) : QDockWidget(parent) {
}

Inspector::~Inspector() {
	QtImGui::destroy();
}

void Inspector::init(Ui::Suede* ui) {
	WinBase::init(ui);

	connect(Hierarchy::instance(), SIGNAL(selectionChanged(const QList<Entity>&, const QList<Entity>&)),
		this, SLOT(onSelectionChanged(const QList<Entity>&, const QList<Entity>&)));

	// 	showView(false);
	// 
	// 	connect(ui_->name, SIGNAL(editingFinished()), this, SLOT(onNameChanged()));
	// 	connect(ui_->tag, SIGNAL(currentIndexChanged(int)), this, SLOT(onTagChanged(int)));
	// 	connect(ui_->active, SIGNAL(stateChanged(int)), this, SLOT(onActiveChanged(int)));
	// 
	// 	connect(ui_->position, SIGNAL(valueChanged(const glm::vec3&)), this, SLOT(onPositionChanged(const glm::vec3&)));
	// 	connect(ui_->rotation, SIGNAL(valueChanged(const glm::vec3&)), this, SLOT(onRotationChanged(const glm::vec3&)));
	// 	connect(ui_->scale, SIGNAL(valueChanged(const glm::vec3&)), this, SLOT(onScaleChanged(const glm::vec3&)));
	// 
	// 	connect(ui_->p0, SIGNAL(clicked()), this, SLOT(onResetButtonClicked()));
	// 	connect(ui_->r0, SIGNAL(clicked()), this, SLOT(onResetButtonClicked()));
	// 	connect(ui_->s0, SIGNAL(clicked()), this, SLOT(onResetButtonClicked()));
}

void Inspector::awake() {
	QtImGui::initialize(ui_->view);
	
	imguiFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("C:/Users/liam.wang/Desktop/tahoma.ttf", 14);

	World::instance()->AddEventListener(this);
}

void Inspector::__updateGL() {
	bool show_test_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImColor(114, 144, 154);

	ui_->view->makeCurrent();

	glewInit();

	QtImGui::newFrame();

	ImGui::PushFont(imguiFont);

	const int windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoTitleBar;
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(35 / 255.f, 38 / 255.f, 41 / 255.f, 1));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(ui_->view->width(), ui_->view->height()), ImGuiCond_FirstUseEver);
	ImGui::Begin("ImGui Demo", nullptr, windowFlags);
	//ImGui::BeginChild("Child", ImVec2(ui_->view->width(), ui_->view->height()), false, windowFlags);

	if (target_) {
		static float f = 0;
		for (int i = 0; i < 20; ++i) {
			ImGui::Text("Hello, world!");
			glm::vec3 pos = target_->GetTransform()->GetPosition();
			if (ImGui::DragFloat3("P", (float*)&pos, 2)) {
				target_->GetTransform()->SetPosition(pos);
			}

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
		}
	}

	//ImGui::EndChild();
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
}

void Inspector::OnWorldEvent(WorldEventBasePointer e) {
	EntityTransformChangedEventPointer tcp = suede_static_cast<EntityTransformChangedEventPointer>(e);
	switch (e->GetEventType()) {
		case WorldEventTypeEntityTransformChanged:
			onEntityTransformChanged(tcp->entity, tcp->prs);
			break;
	}
}

void Inspector::onPositionChanged(const glm::vec3& value) {
	target_->GetTransform()->SetLocalPosition(value);
}

void Inspector::onRotationChanged(const glm::vec3& value) {
	target_->GetTransform()->SetLocalEulerAngles(value);
}

void Inspector::onScaleChanged(const glm::vec3& value) {
	target_->GetTransform()->SetLocalScale(value);
}

void Inspector::onResetButtonClicked() {
	// 	QObject* s = sender();
	// 	if (s == ui_->p0) {
	// 		target_->GetTransform()->SetLocalPosition(glm::vec3(0));
	// 	}
	// 	else if (s == ui_->r0) {
	// 		target_->GetTransform()->SetLocalEulerAngles(glm::vec3(0));
	// 	}
	// 	else {
	// 		target_->GetTransform()->SetLocalScale(glm::vec3(1));
	// 	}
	// 
	// 	drawTransform();
}

void Inspector::onNameChanged() {
	//target_->SetName(ui_->name->text().toStdString());
}

void Inspector::onTagChanged(int index) {
	QString tag;
	if (index >= 0) {
		std::vector<std::string> tags;
		TagManager::instance()->GetAllTags(tags);

		tag = tags[index].c_str();
	}

	target_->SetTag(tag.toStdString());
}

void Inspector::onActiveChanged(int state) {
	target_->SetActiveSelf(!!state);
}

void Inspector::onSelectionChanged(const QList<Entity>& selected, const QList<Entity>& deselected) {
	// TODO: multi-selection.
	if (!selected.empty()) {
		target_ = selected.front();
		redraw();
	}
	else {
		target_ = nullptr;
		showView(false);
	}
}

void Inspector::showView(bool show) {
	const QObjectList& list = ui_->inspectorView->children();
	for (int i = 0; i < list.size(); ++i) {
		if (list[i]->isWidgetType()) {
			((QWidget*)list[i])->setVisible(show);
		}
	}
}

void Inspector::addInspector(CustomInspector* inspector) {
	// 	ui_->content->insertWidget(ui_->content->count() - 1, inspector);
	// 	inspectors_.push_back(inspector);
}

void Inspector::destroyInspectors() {
	qDeleteAll(inspectors_);
	inspectors_.clear();
}

void Inspector::redraw() {
	// 	showView(true);
	// 
	// 	ui_->name->setText(target_->GetName().c_str());
	// 	ui_->active->setChecked(target_->GetActiveSelf());
	// 
	// 	drawTags();
	// 	drawTransform();
	// 	drawInspectors();
}

void Inspector::drawInspectors() {
	destroyInspectors();

	switch (target_->GetType()) {
		case ObjectTypeCamera:
			addInspector(new CameraInspector(target_));
			break;
		case ObjectTypeProjector:
			addInspector(new ProjectorInspector(target_));
			break;
		case ObjectTypeSpotLight:
		case ObjectTypePointLight:
		case ObjectTypeDirectionalLight:
			addInspector(new LightInspector(target_));
			break;
	}

	if (target_->GetMesh()) {
		addInspector(new MeshInspector(target_->GetMesh()));
	}

	if (target_->GetRenderer()) {
		addInspector(new RendererInspector(target_->GetRenderer()));
	}
}

void Inspector::drawTransform() {
	// 	ui_->position->blockSignals(true);
	// 	ui_->position->setValue(target_->GetTransform()->GetLocalPosition());
	// 	ui_->position->blockSignals(false);
	// 
	// 	ui_->rotation->blockSignals(true);
	// 	ui_->rotation->setValue(target_->GetTransform()->GetLocalEulerAngles());
	// 	ui_->rotation->blockSignals(false);
	// 
	// 	ui_->scale->blockSignals(true);
	// 	ui_->scale->setValue(target_->GetTransform()->GetLocalScale());
	// 	ui_->scale->blockSignals(false);
}

#include "time2.h"
void Inspector::drawTags() {
	// 	QStringList items;
	// 	Time::instance()->GetDeltaTime();
	// 	int tagIndex = -1;
	// 	std::vector<std::string> tags;
	// 	TagManager::instance()->GetAllTags(tags);
	// 	for (int i = 0; i < tags.size(); ++i) {
	// 		const std::string& str = tags[i];
	// 		if (tagIndex == -1 && str == target_->GetTag()) {
	// 			tagIndex = i;
	// 		}
	// 
	// 		items.push_back(str.c_str());
	// 	}
	// 
	// 	ui_->tag->blockSignals(true);
	// 
	// 	ui_->tag->clear();
	// 	ui_->tag->addItems(items);
	// 	ui_->tag->setCurrentIndex(tagIndex);
	// 
	// 	ui_->tag->blockSignals(false);
}

void Inspector::onEntityTransformChanged(Entity target, uint prs) {
	if (target == target_ && Math::Highword(prs) == 1) {
		drawTransform();
	}
}
