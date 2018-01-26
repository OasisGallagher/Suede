#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QListView>
#include <QGroupBox>
#include <QPushButton>
#include <QDockWidget>

#include "ui_suede.h"

#include "inspector.h"
#include "hierarchy.h"
#include "tagmanager.h"
#include "tools/math2.h"

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

	connect(Hierarchy::get(), SIGNAL(selectionChanged(const QList<Sprite>&, const QList<Sprite>&)),
		this, SLOT(onSelectionChanged(const QList<Sprite>&, const QList<Sprite>&)));

	showView(false);

	connect(ui_->name, SIGNAL(editingFinished()), this, SLOT(onNameChanged()));
	connect(ui_->tag, SIGNAL(currentIndexChanged(int)), this, SLOT(onTagChanged(int)));
	connect(ui_->active, SIGNAL(stateChanged(int)), this, SLOT(onActiveChanged(int)));

	initTransformUI();

	connect(ui_->text, SIGNAL(editingFinished()), this, SLOT(onTextChanged()));
}

void Inspector::OnWorldEvent(const WorldEventBase* e) {
	switch (e->GetEventType()) {
		case WorldEventTypeSpriteTransformChanged:
			onSpriteTransformChanged((SpriteTransformChangedEvent*)e);
			break;
	}
}

void Inspector::onTransformChanged() {
	QObject* s = sender();
	if (s == transform_[0] || s == transform_[1] || s == transform_[2]) {
		target_->SetLocalPosition(readFields(0));
	}
	else if (s == transform_[3] || s == transform_[4] || s == transform_[5]) {
		target_->SetLocalEulerAngles(readFields(3));
	}
	else {
		target_->SetLocalScale(readFields(6));
	}

	reloadTransform();
}

glm::vec3 Inspector::readFields(int i) {
	return glm::vec3(transform_[i]->text().toFloat(),
		transform_[i + 1]->text().toFloat(),
		transform_[i + 2]->text().toFloat());
}

QString Inspector::float2QString(float f) {
	QString ans;
	ans.setNum(f, 'f', 1);
	int dot = ans.indexOf('.');
	if (dot >= 0 && ans[dot + 1] == '0') {
		ans.remove(dot, 2);
	}

	return ans;
}

void Inspector::writeFields(int i, const glm::vec3& v3) {
	transform_[i]->setText(float2QString(v3.x));
	transform_[i + 1]->setText(float2QString(v3.y));
	transform_[i + 2]->setText(float2QString(v3.z));
}

void Inspector::onResetButtonClicked() {
	QObject* s = sender();
	if (s == reset_[0]) {
		target_->SetLocalPosition(glm::vec3(0));
	}
	else if (s == reset_[1]) {
		target_->SetLocalEulerAngles(glm::vec3(0));
	}
	else {
		target_->SetLocalScale(glm::vec3(1));
	}

	reloadTransform();
}

void Inspector::onNameChanged() {
	target_->SetName(ui_->name->text().toStdString());
}

// TODO: debug
#include <QImage>

void Inspector::onTextChanged() {
	QByteArray arr = ui_->text->text().toLocal8Bit();
	TextMesh textMesh = dsp_cast<TextMesh>(target_->GetMesh());
	textMesh->SetText(arr.toStdString());

	QImage image;
	std::vector<uchar> data;
	if (textMesh->GetFont()->GetTexture()->EncodeToJpg(data) && image.loadFromData(&data[0], data.size())) {
		image.save("e:/1.jpg");
	}
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

void Inspector::onSelectionChanged(const QList<Sprite>& selected, const QList<Sprite>& deselected) {
	if (!selected.empty()) {
		target_ = selected.front();
		reload();
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

void Inspector::reload() {
	showView(true);

	ui_->name->setText(target_->GetName().c_str());
	ui_->active->setChecked(target_->GetActiveSelf());

	reloadTags();
	reloadTransform();

	ui_->mesh->setVisible(!!target_->GetMesh());
	if (target_->GetMesh()) {
		reloadMesh();
	}

	ui_->renderer->setVisible(!!target_->GetRenderer());
	if (target_->GetRenderer()) {
		reloadRenderer();
	}
}

void Inspector::reloadTags() {
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

void Inspector::reloadTransform() {
	for(int i = 0; i < CountOf(transform_); ++i) {
		transform_[i]->blockSignals(true);
	}

	writeFields(0, target_->GetLocalPosition());
	writeFields(3, target_->GetLocalEulerAngles());
	writeFields(6, target_->GetLocalScale());

	for (int i = 0; i < CountOf(transform_); ++i) {
		transform_[i]->blockSignals(false);
	}
}

void Inspector::reloadMesh() {
	Mesh mesh = target_->GetMesh();
	ui_->topology->setText(mesh->GetTopology() == MeshTopologyTriangles ? "Triangles" : "TriangleStrips");
	ui_->subMeshList->clear();

	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		SubMesh subMesh = mesh->GetSubMesh(i);
		uint indexCount, baseIndex, baseVertex;
		subMesh->GetTriangles(indexCount, baseVertex, baseIndex);
		ui_->subMeshList->addItem(QString::asprintf("Index count %d", indexCount));
	}

	ui_->textMeshAttributes->setVisible(mesh->GetType() == ObjectTypeTextMesh);
	if (mesh->GetType() == ObjectTypeTextMesh) {
		TextMesh textMesh = dsp_cast<TextMesh>(mesh);
		ui_->text->setText(QString::fromLocal8Bit(textMesh->GetText().c_str()));

		Font font = textMesh->GetFont();
		ui_->fontName->setText(font->GetFamilyName().c_str());
		ui_->fontSize->setText(QString::number(font->GetFontSize()));
	}
}

void Inspector::reloadRenderer() {

}

void Inspector::onSpriteTransformChanged(SpriteTransformChangedEvent* e) {
	if (e->sprite == target_ && Math::Highword(e->prs) == 0) {
		reloadTransform();
	}
}

void Inspector::initTransformUI() {
	reset_[0] = ui_->p0;
	reset_[1] = ui_->r0;
	reset_[2] = ui_->s0;

	for (int i = 0; i < CountOf(reset_); ++i) {
		connect(reset_[i], SIGNAL(clicked()), this, SLOT(onResetButtonClicked()));
	}

	transform_[0] = ui_->px;
	transform_[1] = ui_->py;
	transform_[2] = ui_->pz;
	transform_[3] = ui_->rx;
	transform_[4] = ui_->ry;
	transform_[5] = ui_->rz;
	transform_[6] = ui_->sx;
	transform_[7] = ui_->sy;
	transform_[8] = ui_->sz;

	QDoubleValidator* validator = new QDoubleValidator(this);
	for (int i = 0; i < CountOf(transform_); ++i) {
		transform_[i]->setValidator(validator);
		connect(transform_[i], SIGNAL(editingFinished()), this, SLOT(onTransformChanged()));
	}
}
