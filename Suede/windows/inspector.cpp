#include <QPixmap>
#include <QLineEdit>
#include <QFormLayout>
#include <QFileDialog>
#include <QProgressBar>
#include <QColorDialog>

#include "ui_suede.h"
#include "labeltexture.h"

#include "variables.h"
#include "inspector.h"
#include "resources.h"
#include "tagmanager.h"
#include "debug/debug.h"
#include "tools/math2.h"
#include "os/filesystem.h"

#define LAYOUT_SPACING	12
#define DEFINE_OBJECT_NAME(name)	static const char* name = #name
namespace ObjectName {
	DEFINE_OBJECT_NAME(colorButton);
	DEFINE_OBJECT_NAME(alphaProgress);
}

static Inspector* inspectorInstance;

struct UserData : public QObjectUserData {
	UserData(uint index, const QString& name, VariantType type) {
		this->index = index;
		this->name = name;
		this->type = type;
	}

	uint index;
	QString name;
	VariantType type;
};

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
	if (s == ui_->px || s == ui_->py || s == ui_->pz) {
		target_->SetLocalPosition(readTransformFields(ui_->px, ui_->py, ui_->pz));
	}
	else if (s == ui_->rx || s == ui_->ry || s == ui_->rz) {
		target_->SetLocalEulerAngles(readTransformFields(ui_->rx, ui_->ry, ui_->rz));
	}
	else {
		target_->SetLocalScale(readTransformFields(ui_->sx, ui_->sy, ui_->sz));
	}

	reloadTransform();
}

glm::vec3 Inspector::readTransformFields(QLineEdit* x, QLineEdit* y, QLineEdit* z) {
	return glm::vec3(x->text().toFloat(), y->text().toFloat(), z->text().toFloat());
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

bool Inspector::isPropertyVisible(const QString& name) {
	return (name == Variables::mainTexture
		|| name == Variables::bumpTexture
		|| name == Variables::specularTexture
		|| name == Variables::emissiveTexture
		|| name == Variables::lightmapTexture
		|| name == Variables::gloss
		|| name == Variables::mainColor
		|| name == Variables::specularColor
		|| name == Variables::emissiveColor);
}

QWidget* Inspector::drawIntField(uint index, const QString& name, int value) {
	QLineEdit* line = new QLineEdit(QString::number(value));
	line->setValidator(new QIntValidator(line));

	line->setUserData(Qt::UserRole, new UserData(index, name, VariantTypeInt));
	connect(line, SIGNAL(editingFinished()), this, SLOT(onEditProperty()));

	return line;
}

QWidget* Inspector::drawFloatField(uint index, const QString& name, float value) {
	QLineEdit* line = new QLineEdit(float2QString(value));
	line->setValidator(new QDoubleValidator(line));

	line->setUserData(Qt::UserRole, new UserData(index, name, VariantTypeFloat));
	connect(line, SIGNAL(editingFinished()), this, SLOT(onEditProperty()));

	return line;
}

QWidget* Inspector::drawTextureField(uint index, const QString& name, Texture value) {
	LabelTexture* label = new LabelTexture;
	label->setFixedSize(32, 32);
	label->setTexture(value);
	label->setUserData(Qt::UserRole, new UserData(index, name, VariantTypeTexture));
	connect(label, SIGNAL(clicked()), this, SLOT(onClickProperty()));
	return label;
}

QWidget* Inspector::drawColor4Field(uint index, const QString& name, const glm::vec4& value) {
	QWidget* widget = new QWidget;
	QVBoxLayout* layout = new QVBoxLayout;
	widget->setLayout(layout);

	LabelTexture* label = new LabelTexture(widget);
	label->setObjectName(ObjectName::colorButton);
	label->setColor(value);
	label->setUserData(Qt::UserRole, new UserData(index, name, VariantTypeColor4));
	connect(label, SIGNAL(clicked()), this, SLOT(onClickProperty()));

	QProgressBar* alpha = new QProgressBar(widget);
	alpha->setObjectName(ObjectName::alphaProgress);
	alpha->setStyleSheet("QProgressBar::chunk { background-color: #595959 }");
	alpha->setFixedHeight(2);
	alpha->setMaximum(255);
	alpha->setValue(int(value.a * 255));

	layout->setSpacing(1);
	layout->setContentsMargins(0, 0, 0, 0);

	layout->addWidget(label);
	layout->addWidget(alpha);

	return widget;
}

QWidget* Inspector::drawVec3Field(uint index, const QString& name, const glm::vec3& value) {
	QWidget* widget = new QWidget;
	QHBoxLayout* layout = new QHBoxLayout(widget);
	layout->setContentsMargins(0, 1, 0, 1);

	widget->setLayout(layout);

	QLineEdit* x = new QLineEdit(float2QString(value.x), widget);
	QLineEdit* y = new QLineEdit(float2QString(value.y), widget);
	QLineEdit* z = new QLineEdit(float2QString(value.z), widget);

	QValidator* validator = new QDoubleValidator(widget);
	x->setValidator(validator);
	y->setValidator(validator);
	z->setValidator(validator);

	// TODO: 3 userData ?
	x->setUserData(Qt::UserRole, new UserData(index, name, VariantTypeVector3));
	connect(x, SIGNAL(editingFinished()), this, SLOT(onEditProperty()));

	y->setUserData(Qt::UserRole, new UserData(index, name, VariantTypeVector3));
	connect(y, SIGNAL(editingFinished()), this, SLOT(onEditProperty()));

	z->setUserData(Qt::UserRole, new UserData(index, name, VariantTypeVector3));
	connect(z, SIGNAL(editingFinished()), this, SLOT(onEditProperty()));

	layout->addWidget(x);
	layout->addWidget(y);
	layout->addWidget(z);

	return widget;
}

QWidget* Inspector::drawVec4Field(uint index, const QString& name, const glm::vec4& value) {
	QWidget* widget = new QWidget;
	QHBoxLayout* layout = new QHBoxLayout(widget);
	layout->setContentsMargins(0, 1, 0, 1);

	widget->setLayout(layout);

	QLineEdit* x = new QLineEdit(float2QString(value.x), widget);
	QLineEdit* y = new QLineEdit(float2QString(value.y), widget);
	QLineEdit* z = new QLineEdit(float2QString(value.z), widget);
	QLineEdit* w = new QLineEdit(float2QString(value.w), widget);

	QValidator* validator = new QDoubleValidator(widget);
	x->setValidator(validator);
	y->setValidator(validator);
	z->setValidator(validator);
	w->setValidator(validator);

	// TODO: 4 userData ?
	x->setUserData(Qt::UserRole, new UserData(index, name, VariantTypeVector4));
	connect(x, SIGNAL(editingFinished()), this, SLOT(onEditProperty()));

	y->setUserData(Qt::UserRole, new UserData(index, name, VariantTypeVector4));
	connect(y, SIGNAL(editingFinished()), this, SLOT(onEditProperty()));

	z->setUserData(Qt::UserRole, new UserData(index, name, VariantTypeVector4));
	connect(z, SIGNAL(editingFinished()), this, SLOT(onEditProperty()));

	w->setUserData(Qt::UserRole, new UserData(index, name, VariantTypeVector4));
	connect(w, SIGNAL(editingFinished()), this, SLOT(onEditProperty()));

	layout->addWidget(x);
	layout->addWidget(y);
	layout->addWidget(z);
	layout->addWidget(w);

	return widget;
}

void Inspector::writeTransformFields(QLineEdit* x, QLineEdit* y, QLineEdit* z, const glm::vec3& v3) {
	x->setText(float2QString(v3.x));
	y->setText(float2QString(v3.y));
	z->setText(float2QString(v3.z));
}

void Inspector::onResetButtonClicked() {
	QObject* s = sender();
	if (s == ui_->p0) {
		target_->SetLocalPosition(glm::vec3(0));
	}
	else if (s == ui_->r0) {
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

void Inspector::onTextChanged() {
	QByteArray arr = ui_->text->text().toLocal8Bit();
	TextMesh textMesh = dsp_cast<TextMesh>(target_->GetMesh());
	textMesh->SetText(arr.toStdString());

	QImage image;
	std::vector<uchar> data;
	if (textMesh->GetFont()->GetTexture()->EncodeToPng(data) && image.loadFromData(&data[0], data.size())) {
		image.save("e:/1.png");
	}
}

void Inspector::onEditProperty() {
	UserData* ud = ((UserData*)sender()->userData(Qt::UserRole));
	uint index = ud->index;
	QString name = ud->name;
	VariantType type = ud->type;

	switch (type) {
		case VariantTypeInt:
			target_->GetRenderer()->GetMaterial(index)->SetInt(name.toStdString(), ((QLineEdit*)sender())->text().toInt());
			break;
		case VariantTypeFloat:
			target_->GetRenderer()->GetMaterial(index)->SetFloat(name.toStdString(), ((QLineEdit*)sender())->text().toFloat());
			break;
		case VariantTypeVector3:
			break;
		case VariantTypeColor3:
			break;
		case VariantTypeColor4:
			break;
		case VariantTypeVector4:
			break;
		case VariantTypeTexture:
			break;
	}
}

void Inspector::onClickProperty() {
	UserData* ud = ((UserData*)sender()->userData(Qt::UserRole));
	uint index = ud->index;
	QString name = ud->name;
	VariantType type = ud->type;

	Material material = target_->GetRenderer()->GetMaterial(index);
	if (type == VariantTypeColor3) {
	}
	else if (type == VariantTypeColor4) {
		glm::ivec4 color = Math::IntColor(material->GetColor4(name.toStdString()));
		QColor old(color.r, color.g, color.b, color.a);
		QColor selected = QColorDialog::getColor(old, this, "Select color", QColorDialog::ShowAlphaChannel);
		if (selected.isValid()) {
			glm::vec4 newColor = Math::NormalizedColor(glm::ivec4(selected.red(), selected.green(), selected.blue(), selected.alpha()));
			material->SetColor4(name.toStdString(), newColor);

			((QWidget*)sender())->setStyleSheet(QString::asprintf("border: 0; background-color: rgb(%d,%d,%d)", selected.red(), selected.green(), selected.blue()));
			QProgressBar* alpha = sender()->parent()->findChild<QProgressBar*>(ObjectName::alphaProgress);
			alpha->setValue(selected.alpha());
		}
	}
	else if (type == VariantTypeTexture) {
		QString path = QFileDialog::getOpenFileName(this, "Select texture", Resources::GetRootDirectory().c_str(), "*.jpg;;*.png");
		if (!path.isEmpty()) {
			Texture2D texture = NewTexture2D();
			QDir dir(Resources::GetRootDirectory().c_str());
			path = dir.relativeFilePath(path);
			texture->Load(path.toStdString());
			material->SetTexture(name.toStdString(), texture);
			((LabelTexture*)sender())->setTexture(texture);
		}
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
	writeTransformFields(ui_->px, ui_->py, ui_->pz, target_->GetLocalPosition());
	writeTransformFields(ui_->rx, ui_->ry, ui_->rz, target_->GetLocalEulerAngles());
	writeTransformFields(ui_->sx, ui_->sy, ui_->sz, target_->GetLocalScale());
}

void Inspector::reloadMesh() {
	Mesh mesh = target_->GetMesh();
	ui_->topology->setText(mesh->GetTopology() == MeshTopologyTriangles ? "Triangles" : "TriangleStrips");
	ui_->vertices->setText(QString::number(mesh->GetVertexCount()));
	ui_->subMeshList->clear();

	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		SubMesh subMesh = mesh->GetSubMesh(i);
		uint indexCount, baseIndex, baseVertex;
		subMesh->GetTriangles(indexCount, baseVertex, baseIndex);

		int triangles = mesh->GetTopology() == MeshTopologyTriangles ? indexCount / 3 : Math::Max(0u, indexCount - 2);
		ui_->subMeshList->addItem(QString::asprintf("Triangles: %d", triangles));
	}

	shrinkListWidget(ui_->subMeshList);

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
	Renderer renderer = target_->GetRenderer();
	ui_->materialList->clear();

	for (int i = 0; i < renderer->GetMaterialCount(); ++i) {
		Material material = renderer->GetMaterial(i);
		ui_->materialList->addItem(material->GetName().c_str());
	}

	shrinkListWidget(ui_->materialList);

	QStringList list;
	const std::vector<ShaderResource>& shaders = Resources::GetShaderResources();
	for (int i = 0; i < shaders.size(); ++i) {
		list << shaders[i].name.c_str();
	}

	qDeleteAll(groups_);
	groups_.clear();

	for (uint materialIndex = 0; materialIndex < renderer->GetMaterialCount(); ++materialIndex) {
		Material material = renderer->GetMaterial(materialIndex);
		Shader shader = material->GetShader();

		QGroupBox* g = new QGroupBox(this);
		groups_.push_back(g);

		g->setTitle(material->GetName().c_str());
		ui_->materialsLayout->addWidget(g);

		QFormLayout* form = new QFormLayout(g);
		g->setLayout(form);

		QComboBox* combo = new QComboBox(g);
		combo->setFocusPolicy(Qt::ClickFocus);

		combo->addItems(list);
		combo->setCurrentIndex(list.indexOf(shader->GetName().c_str()));

		form->addRow("Shader", combo);

		std::vector<const Property*> properties;
		material->GetProperties(properties);

		for (uint i = 0; i < properties.size(); ++i) {
			const Property* p = properties[i];
			if (!isPropertyVisible(p->name.c_str())) {
				continue;
			}

			QWidget* widget = nullptr;
			switch (p->value.GetType()) {
				case VariantTypeInt:
					widget = drawIntField(materialIndex, p->name.c_str(), p->value.GetInt());
					break;
				case VariantTypeFloat:
					widget = drawFloatField(materialIndex, p->name.c_str(), p->value.GetFloat());
					break;
				case VariantTypeColor4:
					widget = drawColor4Field(materialIndex, p->name.c_str(), p->value.GetColor4());
					break;
				case VariantTypeTexture:
					widget = drawTextureField(materialIndex, p->name.c_str(), p->value.GetTexture());
					break;
				case VariantTypeVector3:
					widget = drawVec3Field(materialIndex, p->name.c_str(), p->value.GetVector3());
					break;
				case VariantTypeVector4:
					widget = drawVec4Field(materialIndex, p->name.c_str(), p->value.GetVector4());
					break;
			}

			if (widget != nullptr) {
				const char* ptr = p->name.c_str();
				if (strncmp(ptr, VARIABLE_PREFIX, VARIABLE_PREFIX_LENGTH) == 0) {
					ptr += VARIABLE_PREFIX_LENGTH;
				}

				form->addRow(ptr, widget);
				widget->setParent(g);
			}
		}
	}
}

void Inspector::shrinkListWidget(QListWidget* w) {
	int height = 0;
	for (int i = 0; i < w->count(); ++i) {
		height += w->sizeHintForRow(i);
	}

	w->setFixedHeight(height + 4);
}

void Inspector::onSpriteTransformChanged(SpriteTransformChangedEvent* e) {
	if (e->sprite == target_ && Math::Highword(e->prs) == 0) {
		reloadTransform();
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
