#include "inspector.h"

#include <gl/glew.h>

#include <QFileDialog>

#include "gui.h"
#include "ui_editor.h"

#include "resources.h"
#include "tagmanager.h"
#include "tools/math2.h"
#include "os/filesystem.h"
#include "gui/qtimgui/QtImGui.h"

#include "custom/meshinspector.h"
#include "custom/lightinspector.h"
#include "custom/camerainspector.h"
#include "custom/transforminspector.h"
#include "custom/projectorinspector.h"
#include "custom/meshrendererinspector.h"

Inspector::Inspector(QWidget* parent) : QDockWidget(parent) {
}

Inspector::~Inspector() {
	QtImGui::destroy();

	for (auto p : inspectors_) {
		delete p.second;
	}
}

void Inspector::init(Ui::Editor* ui) {
	WinBase::init(ui);

	connect(Hierarchy::instance(), SIGNAL(selectionChanged(const QList<Entity>&, const QList<Entity>&)),
		this, SLOT(onSelectionChanged(const QList<Entity>&, const QList<Entity>&)));

	addInspector(ObjectType::Transform, new TransformInspector);

	LightInspector* lightInspector = new LightInspector;
	addInspector(ObjectType::PointLight, lightInspector);
	addInspector(ObjectType::DirectionalLight, lightInspector);
	addInspector(ObjectType::SpotLight, lightInspector);

	addInspector(ObjectType::Camera, new CameraInspector);
	addInspector(ObjectType::Projector, new ProjectorInspector);
	addInspector(ObjectType::Mesh, new MeshInspector);
	addInspector(ObjectType::MeshRenderer, new MeshRendererInspector);
}

void Inspector::awake() {
	view_ = new QGLWidget(ui_->inspectorView, Game::instance()->canvas());
	ui_->inspectorViewLayout->addWidget(view_);

	QtImGui::initialize(view_);
	GUI::LoadFont("resources/fonts/tahoma.ttf");

	view_->setFocusPolicy(Qt::StrongFocus);
}

void Inspector::tick() {
	onGui();
}

void Inspector::onGui() {
	QGLContext* oldContext = (QGLContext*)QGLContext::currentContext();
	view_->makeCurrent();

	QtImGui::newFrame();

	// SUEDE TODO: background color and skin.
	glm::vec3 backgroundColor = glm::vec3(35.f, 38.f, 41.f) / 255.f;

	GUI::Begin(view_->width(), view_->height(), backgroundColor);
	if (target_) { drawGui(); }
	GUI::End();

	view_->swapBuffers();
	view_->doneCurrent();

	if (oldContext != nullptr) {
		oldContext->makeCurrent();
	}

	CustomInspector::runMainContextCommands();
}

void Inspector::drawGui() {
	drawBasics();
	drawComponents();
}

void Inspector::drawBasics() {
	bool active = target_->GetActive();
	if (GUI::Toggle("Active", &active)) {
		target_->SetActiveSelf(active);
	}

	GUI::Sameline();

	std::string name = target_->GetName();
	if (GUI::TextField("Name", name)) {
		target_->SetName(name);
	}

	drawTags();
}

void Inspector::onSelectionChanged(const QList<Entity>& selected, const QList<Entity>& deselected) {
	// SUEDE TODO: multi-selection.
	if (!selected.empty()) {
		target_ = selected.front();
	}
	else {
		target_ = nullptr;
	}

	for (auto p : inspectors_) {
		p.second->targetObject(target_);
	}
}

void Inspector::addInspector(ObjectType type, CustomInspector* inspector) {
	inspectors_.push_back(std::make_pair(type, inspector));
}

void Inspector::drawComponents() {
	switch (target_->GetType()) {
		case ObjectType::Camera:
			drawCamera(suede_dynamic_cast<Camera>(target_));
			break;
		case ObjectType::Projector:
			drawProjector(suede_dynamic_cast<Projector>(target_));
			break;
		case ObjectType::SpotLight:
		case ObjectType::PointLight:
		case ObjectType::DirectionalLight:
			drawLight(suede_dynamic_cast<Light>(target_));
			break;
	}

	if (target_->GetMesh()) {
		drawMesh(target_->GetMesh());
	}

	if (target_->GetRenderer()) {
		drawRenderer(target_->GetRenderer());
	}
}

void Inspector::drawLight(Light light) {
	GUI::Separator();
	if (GUI::CollapsingHeader("Light")) {
		GUI::Indent();
		

		GUI::Unindent();
	}
}

void Inspector::drawCamera(Camera camera) {
	if (GUI::CollapsingHeader("Camera")) {
		GUI::Indent();

		

		GUI::Unindent();
	}
}

void Inspector::drawProjector(Projector projector) {
	GUI::Separator();
	if (GUI::CollapsingHeader("Projector")) {
		GUI::Indent();
		GUI::Unindent();
	}
}

void Inspector::drawMesh(Mesh mesh) {
	GUI::Separator();
	if (GUI::CollapsingHeader("Mesh")) {
		GUI::Indent();
		GUI::Unindent();
	}
}

void Inspector::drawRenderer(Renderer renderer) {
	GUI::Separator();
	if (GUI::CollapsingHeader("Renderer")) {
		GUI::Indent();
		

		GUI::Unindent();
	}
}

void Inspector::drawTransform() {
	GUI::Separator();
	if (GUI::CollapsingHeader("Transform")) {
		GUI::Indent();
		

		GUI::Unindent();
	}
}

void Inspector::drawTags() {
	const Tags& tags = TagManager::instance()->GetAllTags();
	int selected = std::find(tags.begin(), tags.end(), target_->GetTag()) - tags.begin();
	if (selected >= tags.size()) { selected = -1; }

	if (GUI::Popup("Tag", &selected, tags.begin(), tags.end())) {
		target_->SetTag(tags[selected]);
	}
}
