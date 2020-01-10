#include "inspectorwindow.h"

#include <gl/glew.h>

#include <QFileDialog>
#include <QMetaProperty>

#include "gui.h"
#include "engine.h"
#include "main/editor.h"
#include "main/selection.h"
#include "materialeditor.h"
#include "main/imguiwidget.h"

#include "tags.h"
#include "resources.h"
#include "math/mathf.h"
#include "os/filesystem.h"
#include "gui/qtimgui/QtImGui.h"

InspectorWindow::InspectorWindow(QWidget* parent) : ChildWindow(parent) {
	addSuedeMetaObject(ObjectType::Transform, std::make_shared<TransformMetaObject>());

	addSuedeMetaObject(ObjectType::Light, std::make_shared<LightMetaObject>());

	addSuedeMetaObject(ObjectType::Camera, std::make_shared<CameraMetaObject>());

	addSuedeMetaObject(ObjectType::Rigidbody, std::make_shared<RigidbodyMetaObject>());

	addSuedeMetaObject(ObjectType::Animation, std::make_shared<AnimationMetaObject>());

	addSuedeMetaObject(ObjectType::ParticleSystem, std::make_shared<ParticleSystemMetaObject>());

	addSuedeMetaObject(ObjectType::Projector, std::make_shared<ProjectorMetaObject>());

	addSuedeMetaObject(ObjectType::TextMesh, std::make_shared<TextMeshMetaObject>());
	addSuedeMetaObject(ObjectType::MeshFilter, std::make_shared<MeshFilterMetaObject>());

	addSuedeMetaObject(ObjectType::MeshRenderer, std::make_shared<MeshRendererMetaObject>());
	addSuedeMetaObject(ObjectType::SkinnedMeshRenderer, std::make_shared<SkinnedMeshRendererMetaObject>());
	addSuedeMetaObject(ObjectType::ParticleRenderer, std::make_shared<ParticleRendererMetaObject>());
}

InspectorWindow::~InspectorWindow() {
}

void InspectorWindow::initUI() {
	view_ = new IMGUIWidget(ui_->inspectorView, editor_->childWindow<GameWindow>()->canvas());
	ui_->inspectorViewLayout->addWidget(view_);

	view_->setForegroundColor(palette().color(foregroundRole()));
	view_->setBackgroundColor(palette().color(backgroundRole()));
}

void InspectorWindow::awake() {
}

void InspectorWindow::tick() {
	onGui();
}

void InspectorWindow::onGui() {
	view_->bind();

	GameObject* selection = editor_->selection()->gameObject();
	if (selection != nullptr) { drawGui(selection); }

	view_->unbind();
}

void InspectorWindow::drawGui(GameObject* go) {
	drawBasics(go);
	drawComponents(go);
}

void InspectorWindow::drawBasics(GameObject* go) {
	bool active = go->GetActive();
	if (GUI::Toggle("Active", active)) {
		go->SetActiveSelf(active);
	}

	GUI::Sameline();

	std::string name = go->GetName();
	if (GUI::TextField("Name", name)) {
		go->SetName(name);
	}

	drawTags(go);
}

void InspectorWindow::addSuedeMetaObject(ObjectType type, std::shared_ptr<ComponentMetaObject> mo) {
	suedeMetaObjects_.insert(std::make_pair(type, mo));
}

void InspectorWindow::drawComponents(GameObject* go) {
	GUI::Indent();
	for (Component* component : go->GetComponents("")) {
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
			GUI::Indent(8);
			drawMetaObject(object);
			GUI::Unindent(8);
		}
	}

	GUI::Unindent();
}

void InspectorWindow::drawTags(GameObject* go) {
	auto& tags_ = Engine::GetSubsystem<Tags>()->GetAllRegisteredTags();
	int selected = std::find(tags_.begin(), tags_.end(), go->GetTag()) - tags_.begin();
	if (selected >= tags_.size()) { selected = -1; }

	if (GUI::Popup("Tag", &selected, tags_.begin(), tags_.end())) {
		go->SetTag(tags_[selected]);
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

void InspectorWindow::drawUserWStringType(QObject* object, const char* name) {
	std::wstring value = object->property(name).value<std::wstring>();
	std::unique_ptr<char[]> str(new char[value.length() * 2 + 1]);
	str[wcstombs(str.get(), value.c_str(), value.length() * 2)] = 0;
	//GUI::TextField()
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
	else if (userType == QMetaTypeId<AnimationWrapMode>::qt_metatype_id()) {
		drawUserEnumType<AnimationWrapMode>(object, name);
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
		GUI::Image(name, texture ? texture->GetNativePointer() : Texture2D::GetBlackTexture()->GetNativePointer());
	}
	else if (userType == QMetaTypeId<std::wstring>::qt_metatype_id()) {
		drawUserWStringType(object, name);
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
