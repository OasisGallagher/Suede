#include "inspector.h"

#include <gl/glew.h>

#include <QFileDialog>
#include <QMetaProperty>

#include "gui.h"
#include "ui_editor.h"
#include "materialeditor.h"

#include "resources.h"
#include "tagmanager.h"
#include "tools/math2.h"
#include "os/filesystem.h"
#include "gui/qtimgui/QtImGui.h"

Inspector::Inspector(QWidget* parent) : QDockWidget(parent) {
}

Inspector::~Inspector() {
	QtImGui::destroy(view_);
}

void Inspector::init(Ui::Editor* ui) {
	WinBase::init(ui);

	connect(Hierarchy::instance(), SIGNAL(selectionChanged(const QList<GameObject>&, const QList<GameObject>&)),
		this, SLOT(onSelectionChanged(const QList<GameObject>&, const QList<GameObject>&)));

	addSuedeMetaObject(ObjectType::Transform, std::make_shared<TransformMetaObject>());

	addSuedeMetaObject(ObjectType::Light, std::make_shared<LightMetaObject>());
	addSuedeMetaObject(ObjectType::Camera, std::make_shared<CameraMetaObject>());

	//addSuedeMetaObject(ObjectType::Projector, std::make_shared<ProjectorInspector>());
	//addSuedeMetaObject(ObjectType::Mesh, std::make_shared<MeshInspector>());
	addSuedeMetaObject(ObjectType::MeshRenderer, std::make_shared<MeshRendererMetaObject>());
	addSuedeMetaObject(ObjectType::SkinnedMeshRenderer, std::make_shared<SkinnedMeshRendererMetaObject>());
}

void Inspector::awake() {
	view_ = new QGLWidget(ui_->inspectorView, Game::instance()->canvas());
	ui_->inspectorViewLayout->addWidget(view_);

	QtImGui::create(view_);
	GUI::LoadFont("resources/fonts/tahoma.ttf");

	view_->setFocusPolicy(Qt::StrongFocus);
	blackTextureID_ = Resources::instance()->GetBlackTexture()->GetNativePointer();
}

void Inspector::tick() {
	onGui();
}

void Inspector::onGui() {
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

void Inspector::drawGui() {
	drawBasics();
	drawComponents();
}

void Inspector::drawBasics() {
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

void Inspector::onSelectionChanged(const QList<GameObject>& selected, const QList<GameObject>& deselected) {
	// SUEDE TODO: multi-selection.
	if (!selected.empty()) {
		target_ = selected.front();
	}
	else {
		target_ = nullptr;
	}
}

void Inspector::addSuedeMetaObject(ObjectType type, std::shared_ptr<ComponentMetaObject> mo) {
	suedeMetaObjects_.insert(std::make_pair(type, mo));
}

void Inspector::drawComponents() {
	GUI::Indent();
	for (Component component : target_->GetComponents(suede_guid(0))) {
		std::string typeName;
		QObject* object = componentMetaObject(component, typeName);
		if (object == nullptr) {
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

void Inspector::drawTags() {
	const Tags& tags = TagManager::instance()->GetAllTags();
	int selected = std::find(tags.begin(), tags.end(), target_->GetTag()) - tags.begin();
	if (selected >= tags.size()) { selected = -1; }

	if (GUI::Popup("Tag", &selected, tags.begin(), tags.end())) {
		target_->SetTag(tags[selected]);
	}
}

void Inspector::drawMetaObject(QObject* object) {
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

void Inspector::drawBuiltinType(const QMetaProperty& p, QObject* object, const char* name) {
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

void Inspector::drawUserType(const QMetaProperty& p, QObject* object, const char* name) {
	int userType = p.userType();
	if (userType == QMetaTypeId<glm::vec2>::qt_metatype_id()) {
		drawUserVectorType(object, name, GUI::Float2Field);
	}
	else if (userType == QMetaTypeId<glm::vec3>::qt_metatype_id()) {
		drawUserVectorType(object, name, GUI::Float3Field);
	}
	else if (userType == QMetaTypeId<glm::vec4>::qt_metatype_id()) {
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
	else if (userType == QMetaTypeId<Material>::qt_metatype_id()) {
		MaterialEditor::draw(object->property(name).value<Material>());
	}
	else if (userType == QMetaTypeId<QVector<Material>>::qt_metatype_id()) {
		drawMaterialVector(object, name);
	}
	else if (userType == QMetaTypeId<RenderTexture>::qt_metatype_id()) {
		RenderTexture texture = object->property(name).value<RenderTexture>();
		GUI::Image(name, texture ? texture->GetNativePointer() : blackTextureID_);
	}
	else {
		Debug::LogError("unable to draw user type %s(%d).", p.typeName(), userType);
	}
}

void Inspector::drawMaterialVector(QObject* object, const char* name) {
	int materialIndex = 0;
	for (Material material : object->property(name).value<QVector<Material>>()) {
		if (materialIndex != 0) { GUI::Separator(); }

		GUI::BeginScope(materialIndex);

		MaterialEditor::draw(material);

		GUI::EndScope();

		++materialIndex;
	}
}

QObject* Inspector::componentMetaObject(Component component, std::string& typeName) {
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
		object = dynamic_cast<QObject*>(component.get());
		typeName = object->metaObject()->className();
	}

	return object;
}
