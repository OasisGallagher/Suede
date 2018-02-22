#include <QPixmap>
#include <QLineEdit>
#include <QFormLayout>
#include <QListWidget>
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
#define MIN_FOV		1
#define MAX_FOV		179

#define DEFINE_STRING_CONSTANT(name)	static const char* name = #name
namespace Constants {
	DEFINE_STRING_CONSTANT(current);

	DEFINE_STRING_CONSTANT(colorButton);
	DEFINE_STRING_CONSTANT(alphaProgress);

	DEFINE_STRING_CONSTANT(cameraFov);
	DEFINE_STRING_CONSTANT(projectorFov);
}

static Inspector* inspectorInstance;

struct UserData : public QObjectUserData {
	UserData(uint index, const QString& name, VariantType type, QWidget* sender = nullptr) {
		this->index = index;
		this->name = name;
		this->type = type;
		this->sender = sender;
	}

	uint index;
	QString name;
	QWidget* sender;
	VariantType type;
};

Inspector* Inspector::get() {
	return inspectorInstance;
}

Inspector::Inspector(QWidget* parent) : QDockWidget(parent) {
	inspectorInstance = this;
	colorPicker_ = new QColorDialog(this);
	colorPicker_->setWindowTitle("Select color");
	colorPicker_->setOption(QColorDialog::NoButtons);
	connect(colorPicker_, SIGNAL(currentColorChanged(const QColor&)), this, SLOT(onColorChanged(const QColor&)));
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
	EntityTransformChangedEventPointer tcp = ssp_cast<EntityTransformChangedEventPointer>(e);
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
	return !name.startsWith(VARIABLE_PREFIX)
		|| name == Variables::mainTexture
		|| name == Variables::bumpTexture
		|| name == Variables::specularTexture
		|| name == Variables::emissiveTexture
		|| name == Variables::lightmapTexture
		|| name == Variables::gloss
		|| name == Variables::mainColor
		|| name == Variables::specularColor
		|| name == Variables::emissiveColor;
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

QWidget* Inspector::drawColorField(uint index, const QString& name, VariantType type, const void* value) {
	QWidget* widget = new QWidget;
	QVBoxLayout* layout = new QVBoxLayout;
	widget->setLayout(layout);

	LabelTexture* label = new LabelTexture(widget);
	label->setObjectName(Constants::colorButton);
	label->setColor(*(glm::vec3*)value);
	label->setUserData(Qt::UserRole, new UserData(index, name, type));
	connect(label, SIGNAL(clicked()), this, SLOT(onClickProperty()));

	layout->setSpacing(1);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(label);

	if (type == VariantTypeColor4) {
		QProgressBar* alpha = new QProgressBar(widget);
		alpha->setObjectName(Constants::alphaProgress);
		alpha->setStyleSheet("QProgressBar::chunk { background-color: #595959 }");
		alpha->setFixedHeight(2);
		alpha->setMaximum(255);
		alpha->setValue(int(((glm::vec4*)value)->a * 255));
		layout->addWidget(alpha);
	}

	return widget;
}

QWidget* Inspector::drawColor3Field(uint index, const QString& name, const glm::vec3& value) {
	return drawColorField(index, name, VariantTypeColor3, &value);
}

QWidget* Inspector::drawColor4Field(uint index, const QString& name, const glm::vec4& value) {
	return drawColorField(index, name, VariantTypeColor4, &value);
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

void Inspector::drawTransformFields(QLineEdit* x, QLineEdit* y, QLineEdit* z, const glm::vec3& v3) {
	x->setText(float2QString(v3.x));
	y->setText(float2QString(v3.y));
	z->setText(float2QString(v3.z));
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

// TODO: debug

void Inspector::onTextChanged() {
	QByteArray arr = ((QLineEdit*)sender())->text().toLocal8Bit();
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

void Inspector::onSliderValueChanged(int value) {
	QSlider* slider = (QSlider*)sender();
	if (slider->objectName() == Constants::cameraFov) {
		dsp_cast<Camera>(target_)->SetFieldOfView(Math::Radians(float(value)));
	}
	else if (slider->objectName() == Constants::projectorFov) {
		dsp_cast<Projector>(target_)->SetFieldOfView(Math::Radians(float(value)));
	}
}

void Inspector::onClickProperty() {
	QWidget* senderWidget = (QWidget*)sender();
	UserData* ud = ((UserData*)senderWidget->userData(Qt::UserRole));
	uint index = ud->index;
	QString name = ud->name;
	VariantType type = ud->type;

	if (type == VariantTypeColor3) {
		onSelectColor3(senderWidget, index, name);
	}
	else if (type == VariantTypeColor4) {
		onSelectColor4(senderWidget, index, name);
	}
	else if (type == VariantTypeTexture) {
		onSelectTexture(senderWidget, index, name);
	}
}

void Inspector::onColorChanged(const QColor& color) {
	QColor selected = colorPicker_->currentColor();
	if (!selected.isValid()) {
		return;
	}

	UserData* data = (UserData*)colorPicker_->userData(Qt::UserRole);
	data->sender->setStyleSheet(QString::asprintf("border: 0; background-color: rgb(%d,%d,%d)",
				selected.red(), selected.green(), selected.blue()));

	Material material = target_->GetRenderer()->GetMaterial(data->index);
	if (data->type == VariantTypeColor4) {
		glm::vec4 newColor = Math::NormalizedColor(glm::ivec4(selected.red(), selected.green(), selected.blue(), selected.alpha()));
		material->SetColor4(data->name.toStdString(), newColor);

		QProgressBar* alpha = data->sender->parent()->findChild<QProgressBar*>(Constants::alphaProgress);
		alpha->setValue(selected.alpha());
	}
	else {
		glm::vec3 newColor = Math::NormalizedColor(glm::ivec3(selected.red(), selected.green(), selected.blue()));
		material->SetColor3(data->name.toStdString(), newColor);
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

void Inspector::onSelectionChanged(const QList<Entity>& selected, const QList<Entity>& deselected) {
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

void Inspector::appendComponentInspector(QWidget* w) {
	ui_->content->insertWidget(ui_->content->count() - 1, w);
	components_.push_back(w);
}

void Inspector::clearComponentInspectors() {
	qDeleteAll(components_);
	components_.clear();
}

void Inspector::redraw() {
	showView(true);

	ui_->name->setText(target_->GetName().c_str());
	ui_->active->setChecked(target_->GetActiveSelf());

	drawTags();
	drawTransform();

	clearComponentInspectors();

	if (target_->GetType() == ObjectTypeCamera) {
		appendComponentInspector(drawCamera(dsp_cast<Camera>(target_)));
	}
	else if (target_->GetType() == ObjectTypeProjector) {
		appendComponentInspector(drawProjector(dsp_cast<Projector>(target_)));
	}

	if (target_->GetMesh()) {
		appendComponentInspector(drawMesh(target_->GetMesh()));
	}

	if (target_->GetRenderer()) {
		appendComponentInspector(drawRenderer(target_->GetRenderer()));
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

QWidget* Inspector::drawCamera(Camera camera) {
	QGroupBox* g = new QGroupBox("Camera", this);

	QFormLayout* form = new QFormLayout(g);
	g->setLayout(form);

	QSlider* slider = new QSlider(g);
	slider->setObjectName(Constants::cameraFov);

	slider->setOrientation(Qt::Horizontal);

	slider->setMinimum(0);
	slider->setMaximum(180);
	slider->setValue(Math::Degrees(camera->GetFieldOfView()));
	connect(slider, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));

	form->addRow(formatRowName("Fov"), slider);

	return g;
}

QWidget* Inspector::drawProjector(Projector projector) {
	QGroupBox* g = new QGroupBox("Projector", this);

	QFormLayout* form = new QFormLayout(g);
	g->setLayout(form);

	QSlider* slider = new QSlider(g);
	slider->setObjectName(Constants::projectorFov);

	slider->setOrientation(Qt::Horizontal);

	slider->setMinimum(0);
	slider->setMaximum(180);
	slider->setValue(projector->GetOrthographicSize());
	connect(slider, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));

	form->addRow(formatRowName("Fov"), slider);

	return g;
}

void Inspector::drawTransform() {
	drawTransformFields(ui_->px, ui_->py, ui_->pz, target_->GetTransform()->GetLocalPosition());
	drawTransformFields(ui_->rx, ui_->ry, ui_->rz, target_->GetTransform()->GetLocalEulerAngles());
	drawTransformFields(ui_->sx, ui_->sy, ui_->sz, target_->GetTransform()->GetLocalScale());
}

QWidget* Inspector::drawMesh(Mesh mesh) {
	QGroupBox* g = new QGroupBox("Mesh", this);
	QFormLayout* form = new QFormLayout(g);

	QLabel* topology = new QLabel(g);
	topology->setText(mesh->GetTopology() == MeshTopologyTriangles ? "Triangles" : "TriangleStrips");
	form->addRow(formatRowName("Topology"), topology);

	QLabel* vertices = new QLabel(g);
	vertices->setText(QString::number(mesh->GetVertices().size()));
	form->addRow(formatRowName("Vertices"), vertices);

	QListWidget* subMeshList = new QListWidget(g);
	
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		SubMesh subMesh = mesh->GetSubMesh(i);
		uint indexCount, baseIndex, baseVertex;
		subMesh->GetTriangles(indexCount, baseVertex, baseIndex);

		int triangles = mesh->GetTopology() == MeshTopologyTriangles ? indexCount / 3 : Math::Max(0u, indexCount - 2);
		subMeshList->addItem(QString::asprintf("Triangles: %d", triangles));
	}

	form->setWidget(form->rowCount(), QFormLayout::SpanningRole, subMeshList);

	shrinkToFit(subMeshList);

	if (mesh->GetType() == ObjectTypeTextMesh) {
		TextMesh textMesh = dsp_cast<TextMesh>(mesh);
		QLineEdit* text = new QLineEdit(g);
		text->setText(QString::fromLocal8Bit(textMesh->GetText().c_str()));
		connect(text, SIGNAL(editingFinished()), this, SLOT(onTextChanged()));
		form->addRow(formatRowName("Text"), text);

		Font font = textMesh->GetFont();
		QLabel* fontName = new QLabel(g);
		fontName->setText(font->GetFamilyName().c_str());
		form->addRow(formatRowName("Font"), fontName);

		QLabel* fontSize = new QLabel(g);
		fontSize->setText(QString::number(font->GetFontSize()));
		form->addRow(formatRowName("Size"), fontSize);
	}

	return g;
}

QWidget* Inspector::drawRenderer(Renderer renderer) {
	QGroupBox* g = new QGroupBox("Renderer", this);
	QFormLayout* form = new QFormLayout(g);

	QListWidget* materialList = new QListWidget(g);

	for (int i = 0; i < renderer->GetMaterialCount(); ++i) {
		Material material = renderer->GetMaterial(i);
		materialList->addItem(material->GetName().c_str());
	}
	form->setWidget(form->rowCount(), QFormLayout::SpanningRole, materialList);
	shrinkToFit(materialList);

	QStringList list;
	const std::vector<ShaderResource>& shaders = Resources::GetShaderResources();
	for (int i = 0; i < shaders.size(); ++i) {
		list << shaders[i].name.c_str();
	}

	QGroupBox* materials = new QGroupBox(formatRowName("Materials"), g);
	QVBoxLayout* materialsLayout = new QVBoxLayout(materials);

	for (uint materialIndex = 0; materialIndex < renderer->GetMaterialCount(); ++materialIndex) {
		drawMaterial(renderer, materialIndex, list, materialsLayout);
	}

	form->setWidget(form->rowCount(), QFormLayout::SpanningRole, materials);

	return g;
}

QString Inspector::formatRowName(const QString& name) {
	QString answer;
	for (int i = 0; i < name.length(); ++i) {
		if (!answer.isEmpty() && name[i].isUpper()) {
			answer += " ";
		}

		if (i == 0 && name[i].isLower()) {
			answer += name[i].toUpper();
		}
		else {
			answer += name[i];
		}
	}

	return answer + ": ";
}

void Inspector::shrinkToFit(QListWidget* w) {
	int height = 0;
	for (int i = 0; i < w->count(); ++i) {
		height += w->sizeHintForRow(i);
	}

	w->setFixedHeight(height + 4);
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

void Inspector::onSelectTexture(QWidget* widget, uint materialIndex, const QString& name) {
	QString path = QFileDialog::getOpenFileName(this, "Select texture", Resources::GetRootDirectory().c_str(), "*.jpg;;*.png");
	if (!path.isEmpty()) {
		Texture2D texture = NewTexture2D();
		QDir dir(Resources::GetRootDirectory().c_str());
		path = dir.relativeFilePath(path);
		texture->Load(path.toStdString());
		Material material = target_->GetRenderer()->GetMaterial(materialIndex);
		material->SetTexture(name.toStdString(), texture);
		((LabelTexture*)sender())->setTexture(texture);
	}
}

void Inspector::onSelectColor3(QWidget* widget, uint materialIndex, const QString& name) {
	colorPicker_->setOption(QColorDialog::ShowAlphaChannel, false);
	Material material = target_->GetRenderer()->GetMaterial(materialIndex);
	glm::ivec3 color = Math::IntColor(material->GetColor3(name.toStdString()));
	QColor old(color.r, color.g, color.b);

	colorPicker_->blockSignals(true);
	colorPicker_->setCurrentColor(old);
	colorPicker_->blockSignals(false);

	delete colorPicker_->userData(Qt::UserRole);
	colorPicker_->setUserData(Qt::UserRole, new UserData(materialIndex, name, VariantTypeColor3, widget));
	colorPicker_->exec();
}

void Inspector::onSelectColor4(QWidget* widget, uint materialIndex, const QString& name) {
	colorPicker_->setOption(QColorDialog::ShowAlphaChannel);
	Material material = target_->GetRenderer()->GetMaterial(materialIndex);
	glm::ivec4 color = Math::IntColor(material->GetColor4(name.toStdString()));
	QColor old(color.r, color.g, color.b, color.a);
	
	colorPicker_->blockSignals(true);
	colorPicker_->setCurrentColor(old);
	colorPicker_->blockSignals(false);

	delete colorPicker_->userData(Qt::UserRole);
	colorPicker_->setUserData(Qt::UserRole, new UserData(materialIndex, name, VariantTypeColor4, widget));
	colorPicker_->exec();
}

void Inspector::drawMaterial(Renderer renderer, uint materialIndex, const QStringList& shaders, QLayout* materialsLayout) {
	Material material = renderer->GetMaterial(materialIndex);
	Shader shader = material->GetShader();

	QGroupBox* g = new QGroupBox(material->GetName().c_str(), this);
	materialsLayout->addWidget(g);

	QFormLayout* form = new QFormLayout(g);
	g->setLayout(form);

	QComboBox* combo = new QComboBox(g);
	combo->setFocusPolicy(Qt::ClickFocus);

	combo->addItems(shaders);
	combo->setCurrentIndex(shaders.indexOf(shader->GetName().c_str()));

	form->addRow(formatRowName("Shader"), combo);

	QWidgetList widgets;
	drawMaterialProperties(widgets, material, materialIndex);

	foreach(QWidget* w, widgets) {
		form->addRow(formatRowName(w->objectName()), w);
		w->setParent(g);
	}
}

void Inspector::drawMaterialProperties(QWidgetList& widgets, Material material, uint materialIndex) {
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
			case VariantTypeColor3:
				widget = drawColor3Field(materialIndex, p->name.c_str(), p->value.GetColor3());
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

			widget->setObjectName(ptr);
			widgets.push_back(widget);
		}
	}
}
