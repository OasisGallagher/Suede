#include "ui_suede.h"
#include "inspector.h"
#include "tagmanager.h"
#include "debug/debug.h"
#include "tools/math2.h"

#include "custom/meshinspector.h"
#include "custom/camerainspector.h"
#include "custom/rendererinspector.h"
#include "custom/projectorinspector.h"

static Inspector* inspectorInstance;

Inspector* Inspector::get() {
	return inspectorInstance;
}

Inspector::Inspector(QWidget* parent) : QDockWidget(parent) {
	inspectorInstance = this;
}

Inspector::~Inspector() {
	inspectorInstance = nullptr;
}

void Inspector::init(Ui::Suede* ui) {
	ChildWindow::init(ui);

	WorldInstance()->AddEventListener(this);

	connect(Hierarchy::get(), SIGNAL(selectionChanged(const QList<Entity>&, const QList<Entity>&)),
		this, SLOT(onSelectionChanged(const QList<Entity>&, const QList<Entity>&)));

	showView(false);

	connect(ui_->name, SIGNAL(editingFinished()), this, SLOT(onNameChanged()));
	connect(ui_->tag, SIGNAL(currentIndexChanged(int)), this, SLOT(onTagChanged(int)));
	connect(ui_->active, SIGNAL(stateChanged(int)), this, SLOT(onActiveChanged(int)));

	initTransformUI();
}

void Inspector::OnWorldEvent(WorldEventBasePointer e) {
	EntityTransformChangedEventPointer tcp = suede_static_cast<EntityTransformChangedEventPointer>(e);
	switch (e->GetEventType()) {
		case WorldEventTypeEntityTransformChanged:
			onEntityTransformChanged(tcp->entity, tcp->prs);
			break;
	}
}

void Inspector::onTransformChanged() {
	QObject* s = sender();
	if (s == ui_->px || s == ui_->py || s == ui_->pz) {
		target_->GetTransform()->SetLocalPosition(readTransformFields(ui_->px, ui_->py, ui_->pz));
	}
	else if (s == ui_->rx || s == ui_->ry || s == ui_->rz) {
		target_->GetTransform()->SetLocalEulerAngles(readTransformFields(ui_->rx, ui_->ry, ui_->rz));
	}
	else {
		target_->GetTransform()->SetLocalScale(readTransformFields(ui_->sx, ui_->sy, ui_->sz));
	}

	drawTransform();
}

glm::vec3 Inspector::readTransformFields(QLineEdit* x, QLineEdit* y, QLineEdit* z) {
	return glm::vec3(x->text().toFloat(), y->text().toFloat(), z->text().toFloat());
}

void Inspector::drawTransformFields(QLineEdit* x, QLineEdit* y, QLineEdit* z, const glm::vec3& v3) {
	x->setText(QString::number(v3.x));
	y->setText(QString::number(v3.y));
	z->setText(QString::number(v3.z));
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
		tag = TagManager::GetAllTags()[index].c_str();
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

	destroyInspectors();

	if (target_->GetType() == ObjectTypeCamera) {
		addInspector(new CameraInspector(target_));
	}
	else if (target_->GetType() == ObjectTypeProjector) {
		addInspector(new ProjectorInspector(target_));
	}

	if (target_->GetMesh()) {
		addInspector(new MeshInspector(target_->GetMesh()));
	}

	if (target_->GetRenderer()) {
		addInspector(new RendererInspector(target_->GetRenderer()));
	}
}

void Inspector::drawTags() {
	QStringList items;

	int tagIndex = -1;
	for (int i = 0; i < TagManager::GetAllTags().size(); ++i) {
		const std::string& str = TagManager::GetAllTags()[i];
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

void Inspector::drawTransform() {
	drawTransformFields(ui_->px, ui_->py, ui_->pz, target_->GetTransform()->GetLocalPosition());
	drawTransformFields(ui_->rx, ui_->ry, ui_->rz, target_->GetTransform()->GetLocalEulerAngles());
	drawTransformFields(ui_->sx, ui_->sy, ui_->sz, target_->GetTransform()->GetLocalScale());
}

void Inspector::onEntityTransformChanged(Entity target, uint prs) {
	if (target == target_ && Math::Highword(prs) == 0) {
		drawTransform();
	}
}

void Inspector::initTransformUI() {
	connect(ui_->p0, SIGNAL(clicked()), this, SLOT(onResetButtonClicked()));
	connect(ui_->r0, SIGNAL(clicked()), this, SLOT(onResetButtonClicked()));
	connect(ui_->s0, SIGNAL(clicked()), this, SLOT(onResetButtonClicked()));

	QDoubleValidator* validator = new QDoubleValidator(this);

#define CONNECT_SIGNAL(line)	line->setValidator(validator); \
	connect(line, SIGNAL(editingFinished()), this, SLOT(onTransformChanged()))

	CONNECT_SIGNAL(ui_->px);
	CONNECT_SIGNAL(ui_->py);
	CONNECT_SIGNAL(ui_->pz);
	CONNECT_SIGNAL(ui_->rx);
	CONNECT_SIGNAL(ui_->ry);
	CONNECT_SIGNAL(ui_->rz);
	CONNECT_SIGNAL(ui_->sx);
	CONNECT_SIGNAL(ui_->sy);
	CONNECT_SIGNAL(ui_->sz);

#undef CONNECT_SIGNAL
}

