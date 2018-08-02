#include "inspector.h"

#include "ui_suede.h"
#include "tagmanager.h"
#include "debug/debug.h"
#include "tools/math2.h"

#include "custom/meshinspector.h"
#include "custom/lightinspector.h"
#include "custom/camerainspector.h"
#include "custom/rendererinspector.h"
#include "custom/projectorinspector.h"

Inspector::Inspector(QWidget* parent) : QDockWidget(parent) {
}

Inspector::~Inspector() {
}

void Inspector::init(Ui::Suede* ui) {
	WinBase::init(ui);

	World::instance()->AddEventListener(this);

	connect(Hierarchy::instance(), SIGNAL(selectionChanged(const QList<Entity>&, const QList<Entity>&)),
		this, SLOT(onSelectionChanged(const QList<Entity>&, const QList<Entity>&)));

	showView(false);

	connect(ui_->name, SIGNAL(editingFinished()), this, SLOT(onNameChanged()));
	connect(ui_->tag, SIGNAL(currentIndexChanged(int)), this, SLOT(onTagChanged(int)));
	connect(ui_->active, SIGNAL(stateChanged(int)), this, SLOT(onActiveChanged(int)));

	connect(ui_->position, SIGNAL(valueChanged(const glm::vec3&)), this, SLOT(onPositionChanged(const glm::vec3&)));
	connect(ui_->rotation, SIGNAL(valueChanged(const glm::vec3&)), this, SLOT(onRotationChanged(const glm::vec3&)));
	connect(ui_->scale, SIGNAL(valueChanged(const glm::vec3&)), this, SLOT(onScaleChanged(const glm::vec3&)));

	connect(ui_->p0, SIGNAL(clicked()), this, SLOT(onResetButtonClicked()));
	connect(ui_->r0, SIGNAL(clicked()), this, SLOT(onResetButtonClicked()));
	connect(ui_->s0, SIGNAL(clicked()), this, SLOT(onResetButtonClicked()));
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
	QObject* s = sender();
	if (s == ui_->p0) {
		target_->GetTransform()->SetLocalPosition(glm::vec3(0));
	}
	else if (s == ui_->r0) {
		target_->GetTransform()->SetLocalEulerAngles(glm::vec3(0));
	}
	else {
		target_->GetTransform()->SetLocalScale(glm::vec3(1));
	}

	drawTransform();
}

void Inspector::onNameChanged() {
	target_->SetName(ui_->name->text().toStdString());
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
	ui_->content->insertWidget(ui_->content->count() - 1, inspector);
	inspectors_.push_back(inspector);
}

void Inspector::destroyInspectors() {
	qDeleteAll(inspectors_);
	inspectors_.clear();
}

void Inspector::redraw() {
	showView(true);

	ui_->name->setText(target_->GetName().c_str());
	ui_->active->setChecked(target_->GetActiveSelf());

	drawTags();
	drawTransform();
	drawInspectors();
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
	ui_->position->blockSignals(true);
	ui_->position->setValue(target_->GetTransform()->GetLocalPosition());
	ui_->position->blockSignals(false);

	ui_->rotation->blockSignals(true);
	ui_->rotation->setValue(target_->GetTransform()->GetLocalEulerAngles());
	ui_->rotation->blockSignals(false);

	ui_->scale->blockSignals(true);
	ui_->scale->setValue(target_->GetTransform()->GetLocalScale());
	ui_->scale->blockSignals(false);
}

#include "time2.h"
void Inspector::drawTags() {
	QStringList items;
	Time::instance()->GetDeltaTime();
	int tagIndex = -1;
	std::vector<std::string> tags;
	TagManager::instance()->GetAllTags(tags);
	for (int i = 0; i < tags.size(); ++i) {
		const std::string& str = tags[i];
		if (tagIndex == -1 && str == target_->GetTag()) {
			tagIndex = i;
		}

		items.push_back(str.c_str());
	}

	ui_->tag->blockSignals(true);

	ui_->tag->clear();
	ui_->tag->addItems(items);
	ui_->tag->setCurrentIndex(tagIndex);

	ui_->tag->blockSignals(false);
}

void Inspector::onEntityTransformChanged(Entity target, uint prs) {
	if (target == target_ && Math::Highword(prs) == 1) {
		drawTransform();
	}
}
