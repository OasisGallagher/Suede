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
	QtImGui::destroy();
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
	for (Component component : target_->GetComponents(0)) {
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

void Inspector::drawBuiltinType(QMetaProperty &p, QObject* object, const char* name) {
	if (p.type() == QMetaType::Bool) {
		bool b = object->property(name).toBool();
		if (GUI::Toggle(name, b)) {
			object->setProperty(name, b);
		}
	}
	else if (p.type() == QMetaType::Float) {
		float f = object->property(name).toFloat();
		if (GUI::FloatField(name, f)) {
			object->setProperty(name, f);
		}
	}
	else if (p.type() == QMetaType::Int) {
		int i = object->property(name).toInt();
		if (GUI::IntField(name, i)) {
			object->setProperty(name, i);
		}
	}
	else if (p.type() == QMetaType::UInt) {
		uint u = object->property(name).toUInt();
		if (GUI::UIntField(name, u)) {
			object->setProperty(name, u);
		}
	}
	else {
		Debug::LogError("unable to draw builtin type %s(%d).", p.typeName(), p.type());
	}
}

void Inspector::drawUserType(QMetaProperty &p, QObject* object, const char* name) {
	int userType = p.userType();
	if (userType == QMetaTypeId<glm::vec2>::qt_metatype_id()) {
		glm::vec2 v2 = object->property(name).value<glm::vec2>();
		if (GUI::Float2Field(name, v2)) {
			object->setProperty(name, QVariant::fromValue(v2));
		}
	}
	else if (userType == QMetaTypeId<glm::vec3>::qt_metatype_id()) {
		glm::vec3 v3 = object->property(name).value<glm::vec3>();
		if (GUI::Float3Field(name, v3)) {
			object->setProperty(name, QVariant::fromValue(v3));
		}
	}
	else if (userType == QMetaTypeId<glm::vec4>::qt_metatype_id()) {
		glm::vec4 v4 = object->property(name).value<glm::vec4>();
		if (GUI::Float4Field(name, v4)) {
			object->setProperty(name, QVariant::fromValue(v4));
		}
	}
	else if (userType == QMetaTypeId<ClearType>::qt_metatype_id()) {
		int selected = -1;
		ClearType type = object->property(name).value<ClearType>();
		if (GUI::EnumPopup(name, +type, selected)) {
			object->setProperty(name, QVariant::fromValue(ClearType::value(selected)));
		}
	}
	else if (userType == QMetaTypeId<RenderPath>::qt_metatype_id()) {
		int selected = -1;
		RenderPath path = object->property(name).value<RenderPath>();
		if (GUI::EnumPopup(name, +path, selected)) {
			object->setProperty(name, QVariant::fromValue(RenderPath::value(selected)));
		}
	}
	else if (userType == QMetaTypeId<DepthTextureMode>::qt_metatype_id()) {
		int selected = -1;
		DepthTextureMode mode = object->property(name).value<DepthTextureMode>();
		if (GUI::EnumPopup(name, +mode, selected)) {
			object->setProperty(name, QVariant::fromValue(DepthTextureMode::value(selected)));
		}
	}
	else if (userType == QMetaTypeId<LightType>::qt_metatype_id()) {
		int selected = -1;
		LightType type = object->property(name).value<LightType>();
		if (GUI::EnumPopup(name, +type, selected)) {
			object->setProperty(name, QVariant::fromValue(LightType::value(selected)));
		}
	}
	else if (userType == QMetaTypeId<Color>::qt_metatype_id()) {
		Color color = object->property(name).value<Color>();
		if (GUI::ColorField(name, color)) {
			object->setProperty(name, QVariant::fromValue(color));
		}
	}
	else if (userType == QMetaTypeId<Material>::qt_metatype_id()) {
		MaterialEditor::draw(object->property(name).value<Material>());
	}
	else if (userType == QMetaTypeId<QVector<Material>>::qt_metatype_id()) {
		bool first = true;
		for (Material material : object->property(name).value<QVector<Material>>()) {
			if (!first) { GUI::Separator(); }
			MaterialEditor::draw(material);
			first = false;
		}
	}
	else if (userType == QMetaTypeId<RenderTexture>::qt_metatype_id()) {
		static uint blackTextureID = Resources::instance()->GetBlackTexture()->GetNativePointer();
		RenderTexture texture = object->property(name).value<RenderTexture>();
		GUI::Image(name, texture ? texture->GetNativePointer() : blackTextureID);
	}
	else if (userType == QMetaTypeId<Rect>::qt_metatype_id()) {
		Rect rect = object->property(name).value<Rect>();
		if (GUI::Float4Field(name, *(glm::vec4*)&rect)) {
			object->setProperty(name, QVariant::fromValue(rect));
		}
	}
	else if (userType == QMetaTypeId<iranged>::qt_metatype_id()) {
		iranged rint = object->property(name).value<iranged>();
		int i = rint.value();
		if (GUI::IntSlider(name, i, rint.min(), rint.max())) {
			object->setProperty(name, QVariant::fromValue(rint = i));
		}
	}
	else if (userType == QMetaTypeId<franged>::qt_metatype_id()) {
		franged rfloat = object->property(name).value<franged>();
		float f = rfloat.value();
		if (GUI::Slider(name, f, rfloat.min(), rfloat.max())) {
			object->setProperty(name, QVariant::fromValue(rfloat = f));
		}
	}
	else {
		Debug::LogError("unable to draw user type %s(%d).", p.typeName(), userType);
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
