#include "inspector.h"

#include <gl/glew.h>

#include <QFileDialog>

#include "gui/gui.h"
#include "ui_suede.h"

#include "resources.h"
#include "tagmanager.h"
#include "tools/math2.h"

Inspector::Inspector(QWidget* parent) : QDockWidget(parent) {
}

Inspector::~Inspector() {
	GUI::destroy();
}

void Inspector::init(Ui::Suede* ui) {
	WinBase::init(ui);

	connect(Hierarchy::instance(), SIGNAL(selectionChanged(const QList<Entity>&, const QList<Entity>&)),
		this, SLOT(onSelectionChanged(const QList<Entity>&, const QList<Entity>&)));
}

void Inspector::awake() {
	view_ = new QGLWidget(ui_->inspectorView, Game::instance()->canvas());
	ui_->inspectorViewLayout->addWidget(view_);

	GUI::initialize(view_);
	view_->setFocusPolicy(Qt::StrongFocus);
}

void Inspector::tick() {
	onGui();
}

void Inspector::onGui() {
	QGLContext* oldContext = (QGLContext*)QGLContext::currentContext();
	view_->makeCurrent();
	
	GUI::begin();
	if (target_) { drawGui(); }
	GUI::end();

	view_->swapBuffers();
	view_->doneCurrent();

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
	if (GUI::checkbox("Active", &active)) {
		target_->SetActiveSelf(active);
	}

	GUI::sameline();

	std::string name = target_->GetName();
	if (GUI::input("Name", name)) {
		target_->SetName(name);
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
			drawCamera(suede_dynamic_cast<Camera>(target_));
			break;
		case ObjectTypeProjector:
			drawProjector(suede_dynamic_cast<Projector>(target_));
			break;
		case ObjectTypeSpotLight:
		case ObjectTypePointLight:
		case ObjectTypeDirectionalLight:
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
	GUI::separator();
	if (GUI::collapsingHeader("Light")) {
	}
}

void Inspector::drawCamera(Camera camera) {
	if (GUI::collapsingHeader("Camera")) {
		int selected = -1;
		if (GUI::enums("Clear Type", +camera->GetClearType(), selected)) {
			camera->SetClearType(ClearType::value(selected));
		}

		float fieldOfView = Math::Degrees(camera->GetFieldOfView());
		if (GUI::slider("FOV", &fieldOfView, 1, 179)) {
			camera->SetFieldOfView(Math::Radians(fieldOfView));
		}

		float nearClipPlane = camera->GetNearClipPlane();
		if (GUI::single("Near", &nearClipPlane)) {
			camera->SetNearClipPlane(nearClipPlane);
		}

		float farClipPlane = camera->GetFarClipPlane();
		if (GUI::single("Far", &farClipPlane)) {
			camera->SetFarClipPlane(farClipPlane);
		}
	}
}

void Inspector::drawProjector(Projector projector) {
	GUI::separator();
	if (GUI::collapsingHeader("Projector")) {
	}
}

void Inspector::drawMesh(Mesh mesh) {
	GUI::separator();
	if (GUI::collapsingHeader("Mesh")) {
	}
}

void Inspector::drawRenderer(Renderer renderer) {
	GUI::separator();
	if (GUI::collapsingHeader("Renderer")) {
		for (Material material : renderer->GetMaterials()) {
			drawMaterial(material);
		}
	}
}

void Inspector::drawMaterial(Material material) {
	std::vector<const Property*> properties;
	material->GetProperties(properties);

	for (const Property* p : properties) {
		switch (p->value.GetType()) {
			case VariantTypeFloat:
				drawSingle(material, p);
				break;
			case VariantTypeVector3:
				drawSingle3(material, p);
				break;
			case VariantTypeVector4:
				drawSingle4(material, p);
				break;
			case VariantTypeColor3:
				drawColor3(material, p);
				break;
			case VariantTypeColor4:
				drawColor4(material, p);
				break;
			case VariantTypeTexture:
				drawTexture(material, p);
				break;
		}
	}
}

void Inspector::drawTexture(Material material, const Property* p) {
	Texture texture = material->GetTexture(p->name);
	if (GUI::imageButton(p->name.c_str(), texture->GetNativePointer())) {
		QString path = QFileDialog::getOpenFileName(this, tr("SelectTexture"), Resources::instance()->GetTextureDirectory().c_str(), "*.jpg;;*.png");
		if (!path.isEmpty()) {
			Texture2D newTexture = NewTexture2D();
			path = QDir(Resources::instance()->GetTextureDirectory().c_str()).relativeFilePath(path);

			if (newTexture->Create(path.toStdString())) {
				material->SetTexture(p->name, newTexture);
			}
		}
	}
}

void Inspector::drawColor3(Material material, const Property* p) {
	glm::vec3 value = material->GetColor3(p->name);
	if (GUI::color3(p->name.c_str(), (float*)&value)) {
		material->SetColor3(p->name, value);
	}
}

void Inspector::drawColor4(Material material, const Property* p) {
	glm::vec4 value = material->GetColor4(p->name);
	if (GUI::color4(p->name.c_str(), (float*)&value)) {
		material->SetColor4(p->name, value);
	}
}

void Inspector::drawSingle(Material material, const Property* p) {
	float value = material->GetFloat(p->name);
	if (GUI::single(p->name.c_str(), &value)) {
		material->SetFloat(p->name, value);
	}
}

void Inspector::drawSingle3(Material material, const Property* p) {
	glm::vec3 value = material->GetVector3(p->name);
	if (GUI::single3(p->name.c_str(), (float*)&value)) {
		material->SetVector3(p->name, value);
	}
}

void Inspector::drawSingle4(Material material, const Property* p) {
	glm::vec4 value = material->GetVector4(p->name);
	if (GUI::single4(p->name.c_str(), (float*)&value)) {
		material->SetVector4(p->name, value);
	}
}

void Inspector::drawTransform() {
	GUI::separator();
	if (GUI::collapsingHeader("Transform")) {
		glm::vec3 v3 = target_->GetTransform()->GetLocalPosition();
		if (GUI::single3("P", (float*)&v3)) {
			target_->GetTransform()->SetPosition(v3);
		}

		v3 = target_->GetTransform()->GetLocalEulerAngles();
		if (GUI::single3("R", (float*)&v3)) {
			target_->GetTransform()->SetLocalEulerAngles(v3);
		}

		v3 = target_->GetTransform()->GetLocalScale();
		if (GUI::single3("S", (float*)&v3)) {
			target_->GetTransform()->SetLocalScale(v3);
		}
	}
}

void Inspector::drawTags() {
	const Tags& tags = TagManager::instance()->GetAllTags();
	int selected = std::find(tags.begin(), tags.end(), target_->GetTag()) - tags.begin();
	if (selected >= tags.size()) { selected = -1; }

	if (GUI::combo("Tag", &selected, tags.begin(), tags.end())) {
		target_->SetTag(tags[selected]);
	}
}
