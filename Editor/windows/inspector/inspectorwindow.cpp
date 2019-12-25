#include "inspectorwindow.h"

#include <gl/glew.h>

#include <QFileDialog>
#include <QMetaProperty>

#include "gui.h"
#include "main/editor.h"
#include "materialeditor.h"

#include "resources.h"
#include "tagmanager.h"
#include "math/mathf.h"
#include "os/filesystem.h"
#include "gui/qtimgui/QtImGui.h"

InspectorWindow::InspectorWindow(QWidget* parent) : ChildWindow(parent) {
	addSuedeMetaObject(ObjectType::Transform, std::make_shared<TransformMetaObject>());

	addSuedeMetaObject(ObjectType::Light, std::make_shared<LightMetaObject>());
	addSuedeMetaObject(ObjectType::Camera, std::make_shared<CameraMetaObject>());

	addSuedeMetaObject(ObjectType::Rigidbody, std::make_shared<RigidbodyMetaObject>());

	addSuedeMetaObject(ObjectType::Projector, std::make_shared<ProjectorMetaObject>());
	addSuedeMetaObject(ObjectType::MeshFilter, std::make_shared<MeshFilterMetaObject>());
	addSuedeMetaObject(ObjectType::MeshRenderer, std::make_shared<MeshRendererMetaObject>());
	addSuedeMetaObject(ObjectType::SkinnedMeshRenderer, std::make_shared<SkinnedMeshRendererMetaObject>());
}

InspectorWindow::~InspectorWindow() {
	QtImGui::destroy(view_);
}

void InspectorWindow::initUI() {
	connect(editor_->childWindow<HierarchyWindow>(),
		SIGNAL(selectionChanged(const QList<GameObject*>&, const QList<GameObject*>&)),
		this, SLOT(onSelectionChanged(const QList<GameObject*>&, const QList<GameObject*>&))
	);

	view_ = new QGLWidget(ui_->inspectorView, editor_->childWindow<GameWindow>()->canvas());
	ui_->inspectorViewLayout->addWidget(view_);
	view_->setFocusPolicy(Qt::StrongFocus);
}

void InspectorWindow::awake() {
	QtImGui::create(view_);
	GUI::LoadFont("resources/fonts/tahoma.ttf");

	blackTextureID_ = Texture2D::GetBlackTexture()->GetNativePointer();
}

void InspectorWindow::tick() {
	onGui();
}

void InspectorWindow::onGui() {
	QGLContext* oldContext = (QGLContext*)QGLContext::currentContext();
	view_->makeCurrent();

	QtImGui::newFrame(view_);

	QColor foregrouldColor = palette().color(foregroundRole());
	QColor backgroundColor = palette().color(backgroundRole());

	GUI::Begin(view_->width(), view_->height(), 
		Color(foregrouldColor.redF(), foregrouldColor.greenF(), foregrouldColor.blueF()),
		Color(backgroundColor.redF(), backgroundColor.greenF(), backgroundColor.blueF())
	);

	if (target_) { drawGui(); }

	GUI::End();

	view_->swapBuffers();
	view_->doneCurrent();

	if (oldContext != nullptr) {
		oldContext->makeCurrent();
	}

	MaterialEditor::runMainContextCommands();
}

void InspectorWindow::drawGui() {
	drawBasics();
	drawComponents();
}

void InspectorWindow::drawBasics() {
	bool active = target_->GetActive();
	if (GUI::Toggle("Active", active)) {
		target_->SetActiveSelf(active);
	}

	GUI::Sameline();

	std::string name = target_->GetName();
	if (GUI::TextField("Name", name)) {
		target_->SetName(name);
	}

	drawTags();
}

void InspectorWindow::onSelectionChanged(const QList<GameObject*>& selected, const QList<GameObject*>& deselected) {
	// SUEDE TODO: multi-selection.
	if (!selected.empty()) {
		target_ = selected.front();
	}
	else {
		target_ = nullptr;
	}
}

void InspectorWindow::addSuedeMetaObject(ObjectType type, std::shared_ptr<ComponentMetaObject> mo) {
	suedeMetaObjects_.insert(std::make_pair(type, mo));
}

void InspectorWindow::drawComponents() {
	GUI::Indent();
	for (Component* component : target_->GetComponents("")) {
		std::string typeName;
		QObject* object = componentMetaObject(component, typeName);
		if (object == nullptr) {
			Debug::LogWarning(("can not find meta object of type " + typeName).c_str());
			continue;
		}

		bool enabled = component->GetEnabled();

		if (GUI::Toggle(("##" + typeName).c_str(), enabled)) {
			component->SetEnabled(enabled);
		}

		GUI::Sameline();

		if (GUI::CollapsingHeader(typeName.c_str())) {
			GUI::Indent();
			drawMetaObject(object);
			GUI::Unindent();
		}
	}

	GUI::Unindent();
}

void InspectorWindow::drawTags() {
	auto& tags = TagManager::GetAllRegisteredTags();
	int selected = std::find(tags.begin(), tags.end(), target_->GetTag()) - tags.begin();
	if (selected >= tags.size()) { selected = -1; }

	if (GUI::Popup("Tag", &selected, tags.begin(), tags.end())) {
		target_->SetTag(tags[selected]);
	}
}

void InspectorWindow::drawMetaObject(QObject* object) {
	const QMetaObject* meta = object->metaObject();
	for (int i = 0; i < meta->propertyCount(); ++i) {
		QMetaProperty p = meta->property(i);
		const char* name = p.name();
		if (strcmp(name, "objectName") == 0) { continue; }
		if (p.type() != QMetaType::User) {
			drawBuiltinType(p, object, name);
		}
		else {
			drawUserType(p, object, name);
		}
	}
}

void InspectorWindow::drawBuiltinType(const QMetaProperty& p, QObject* object, const char* name) {
	QVariant::Type type = p.type();
	if (type == QMetaType::Bool) {
		bool b = object->property(name).toBool();
		if (GUI::Toggle(name, b)) {
			object->setProperty(name, b);
		}
	}
	else if (type == QMetaType::Float) {
		drawBuiltinType(object, name, GUI::FloatField);
	}
	else if (type == QMetaType::Int) {
		drawBuiltinType(object, name, GUI::IntField);
	}
	else if (type == QMetaType::UInt) {
		drawBuiltinType(object, name, GUI::UIntField);
	}
	else {
		Debug::LogError("unable to draw builtin type %s(%d).", p.typeName(), type);
	}
}

void InspectorWindow::drawUserType(const QMetaProperty& p, QObject* object, const char* name) {
	int userType = p.userType();
	if (userType == QMetaTypeId<Vector2>::qt_metatype_id()) {
		drawUserVectorType(object, name, GUI::Float2Field);
	}
	else if (userType == QMetaTypeId<Vector3>::qt_metatype_id()) {
		drawUserVectorType(object, name, GUI::Float3Field);
	}
	else if (userType == QMetaTypeId<Vector4>::qt_metatype_id()) {
		drawUserVectorType(object, name, GUI::Float4Field);
	}
	else if (userType == QMetaTypeId<Color>::qt_metatype_id()) {
		drawUserVectorType(object, name, GUI::ColorField);
	}
	else if (userType == QMetaTypeId<Rect>::qt_metatype_id()) {
		drawUserVectorType(object, name, GUI::NormalizedRectField);
	}
	else if (userType == QMetaTypeId<ClearType>::qt_metatype_id()) {
		drawUserEnumType<ClearType>(object, name);
	}
	else if (userType == QMetaTypeId<RenderPath>::qt_metatype_id()) {
		drawUserEnumType<RenderPath>(object, name);
	}
	else if (userType == QMetaTypeId<DepthTextureMode>::qt_metatype_id()) {
		drawUserEnumType<DepthTextureMode>(object, name);
	}
	else if (userType == QMetaTypeId<LightType>::qt_metatype_id()) {
		drawUserEnumType<LightType>(object, name);
	}
	else if (userType == QMetaTypeId<LightImportance>::qt_metatype_id()) {
		drawUserEnumType<LightImportance>(object, name);
	}
	else if (userType == QMetaTypeId<iranged>::qt_metatype_id()) {
		drawUserRangeType(object, name, GUI::IntSlider);
	}
	else if (userType == QMetaTypeId<franged>::qt_metatype_id()) {
		drawUserRangeType(object, name, GUI::Slider);
	}
	else if (userType == QMetaTypeId<Material*>::qt_metatype_id()) {
		MaterialEditor::draw(object->property(name).value<Material*>());
	}
	else if (userType == QMetaTypeId<QVector<Material*>>::qt_metatype_id()) {
		drawMaterialVector(object, name);
	}
	else if (userType == QMetaTypeId<RenderTexture*>::qt_metatype_id()) {
		RenderTexture* texture = object->property(name).value<RenderTexture*>();
		GUI::Image(name, texture ? texture->GetNativePointer() : blackTextureID_);
	}
	else {
		Debug::LogError("unable to draw user type %s(%d).", p.typeName(), userType);
	}
}

void InspectorWindow::drawMaterialVector(QObject* object, const char* name) {
	int materialIndex = 0;
	for (Material* material : object->property(name).value<QVector<Material*>>()) {
		if (materialIndex != 0) { GUI::Separator(); }

		GUI::BeginScope(materialIndex);

		MaterialEditor::draw(material);

		GUI::EndScope();

		++materialIndex;
	}
}

QObject* InspectorWindow::componentMetaObject(Component* component, std::string& typeName) {
	QObject* object = nullptr;
	ObjectType type = component->GetObjectType();
	if (type != ObjectType::CustomBehaviour) {
		auto pos = suedeMetaObjects_.find(type);
		if (pos != suedeMetaObjects_.end()) {
			pos->second->setComponent(component);
			object = pos->second.get();
		}

		typeName = type.to_string();
	}
	else {
		object = dynamic_cast<QObject*>(component);
		typeName = object->metaObject()->className();
	}

	return object;
}
