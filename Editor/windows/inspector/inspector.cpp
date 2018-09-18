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
}

void Inspector::init(Ui::Editor* ui) {
	WinBase::init(ui);

	connect(Hierarchy::instance(), SIGNAL(selectionChanged(const QList<GameObject>&, const QList<GameObject>&)),
		this, SLOT(onSelectionChanged(const QList<GameObject>&, const QList<GameObject>&)));

	addInspector(ObjectType::Transform, std::make_shared<TransformInspector>());

	auto lightInspector = std::make_shared<LightInspector>();
	addInspector(ObjectType::PointLight, lightInspector);
	addInspector(ObjectType::DirectionalLight, lightInspector);
	addInspector(ObjectType::SpotLight, lightInspector);

	addInspector(ObjectType::Camera, std::make_shared<CameraInspector>());
	addInspector(ObjectType::Projector, std::make_shared<ProjectorInspector>());
	addInspector(ObjectType::Mesh, std::make_shared<MeshInspector>());
	addInspector(ObjectType::MeshRenderer, std::make_shared<MeshRendererInspector>());
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

void Inspector::onSelectionChanged(const QList<GameObject>& selected, const QList<GameObject>& deselected) {
	// SUEDE TODO: multi-selection.
	if (!selected.empty()) {
		target_ = selected.front();
	}
	else {
		target_ = nullptr;
	}
}

void Inspector::addInspector(ObjectType type, std::shared_ptr<CustomInspector> inspector) {
	inspectors_.insert(std::make_pair(type, inspector));
}

#include <QMetaProperty>
#include "../game/testbehaviour.h"
#include "custom/componentmetaobject.h"

void Inspector::drawComponents() {
	for (Component component : target_->GetComponents(0)) {
		ObjectType type = component->GetObjectType();
		QObject* object = nullptr;
		if (type != ObjectType::CustomBehaviour) {
			if (type == ObjectType::Camera) {
				CameraMetaObject* cmo = new CameraMetaObject;
				cmo->setComponent(component);
				object = cmo;
			}
		}
		else {
			object = dynamic_cast<QObject*>(component.get());
		}

		if (object == nullptr) { continue; }

		const QMetaObject* meta = object->metaObject();
		for (int i = 0; i < meta->propertyCount(); ++i) {
			QMetaProperty p = meta->property(i);
			const char* n = p.name();
			if (strcmp(n, "objectName") == 0) { continue; }
			if (p.type() != QMetaType::User) {
				if (p.type() == QMetaType::Float) {
					float f = object->property(n).toFloat();
					f = 0;
				}
			}
			else {
				int userType = p.userType();
				if (userType == QMetaTypeId<SuedeObject>::qt_metatype_id()) {
					SuedeObject obj = object->property(n).value<SuedeObject>();
					obj->name = "meta_suede";
				}
				else if (userType == QMetaTypeId<ClearType>::qt_metatype_id()) {
					ClearType type = object->property(n).value<ClearType>();
					type = ClearType::Color;
				}
				else if (userType == QMetaTypeId<Color>::qt_metatype_id()) {
					Color color = object->property(n).value<Color>();
					color = Color::black;
				}
			}
		}

		//auto pos = inspectors_.find(type);
		//if (pos != inspectors_.end()) {
		//	pos->second->onGui(component);
		//}
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
