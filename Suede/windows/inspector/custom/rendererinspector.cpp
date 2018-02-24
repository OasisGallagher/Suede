#include <QGroupBox>
#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QFormLayout>
#include <QFileDialog>
#include <QProgressBar>

#include "tools/math2.h"

#include "renderer.h"
#include "variables.h"
#include "resources.h"
#include "rendererinspector.h"
#include "../../controls/labeltexture.h"

namespace Constants {
	static uint layoutSpacing = 12;
}

#define DEFINE_LITERAL(name)	static const char* name = #name
namespace Literals {
	DEFINE_LITERAL(current);

	DEFINE_LITERAL(colorButton);
	DEFINE_LITERAL(alphaProgress);
}

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

RendererInspector::RendererInspector(Object object) : CustomInspector("Renderer", object) {
	colorPicker_ = new QColorDialog(this);
	colorPicker_->setWindowTitle("Select color");
	colorPicker_->setOption(QColorDialog::NoButtons);
	connect(colorPicker_, SIGNAL(currentColorChanged(const QColor&)), this, SLOT(onColorChanged(const QColor&)));

	Renderer renderer = dsp_cast<Renderer>(object_);
	QListWidget* materialList = new QListWidget(this);

	for (int i = 0; i < renderer->GetMaterialCount(); ++i) {
		Material material = renderer->GetMaterial(i);
		materialList->addItem(material->GetName().c_str());
	}
	form_->setWidget(form_->rowCount(), QFormLayout::SpanningRole, materialList);
	shrinkToFit(materialList);

	QStringList list;
	const std::vector<ShaderResource>& shaders = Resources::GetShaderResources();
	for (int i = 0; i < shaders.size(); ++i) {
		list << shaders[i].name.c_str();
	}

	QGroupBox* materials = new QGroupBox(formatRowName("Materials"), this);
	QVBoxLayout* materialsLayout = new QVBoxLayout(materials);

	for (uint materialIndex = 0; materialIndex < renderer->GetMaterialCount(); ++materialIndex) {
		drawMaterial(renderer, materialIndex, list, materialsLayout);
	}

	form_->setWidget(form_->rowCount(), QFormLayout::SpanningRole, materials);
}

void RendererInspector::drawMaterial(Renderer renderer, uint materialIndex, const QStringList& shaders, QLayout* materialsLayout) {
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

void RendererInspector::drawMaterialProperties(QWidgetList& widgets, Material material, uint materialIndex) {
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

bool RendererInspector::isPropertyVisible(const QString& name) {
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

QWidget* RendererInspector::drawIntField(uint index, const QString& name, int value) {
	QLineEdit* line = new QLineEdit(QString::number(value));
	line->setValidator(new QIntValidator(line));

	line->setUserData(Qt::UserRole, new UserData(index, name, VariantTypeInt));
	connect(line, SIGNAL(editingFinished()), this, SLOT(onEditProperty()));

	return line;
}

QWidget* RendererInspector::drawFloatField(uint index, const QString& name, float value) {
	QLineEdit* line = new QLineEdit(float2QString(value));
	line->setValidator(new QDoubleValidator(line));

	line->setUserData(Qt::UserRole, new UserData(index, name, VariantTypeFloat));
	connect(line, SIGNAL(editingFinished()), this, SLOT(onEditProperty()));

	return line;
}

QWidget* RendererInspector::drawTextureField(uint index, const QString& name, Texture value) {
	LabelTexture* label = new LabelTexture;
	label->setFixedSize(32, 32);
	label->setTexture(value);
	label->setUserData(Qt::UserRole, new UserData(index, name, VariantTypeTexture));
	connect(label, SIGNAL(clicked()), this, SLOT(onEditProperty()));
	return label;
}

QWidget* RendererInspector::drawColorField(uint index, const QString& name, VariantType type, const void* value) {
	QWidget* widget = new QWidget;
	QVBoxLayout* layout = new QVBoxLayout;
	widget->setLayout(layout);

	LabelTexture* label = new LabelTexture(widget);
	label->setObjectName(Literals::colorButton);
	label->setColor(*(glm::vec3*)value);
	label->setUserData(Qt::UserRole, new UserData(index, name, type));
	connect(label, SIGNAL(clicked()), this, SLOT(onEditProperty()));

	layout->setSpacing(1);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(label);

	if (type == VariantTypeColor4) {
		QProgressBar* alpha = new QProgressBar(widget);
		alpha->setObjectName(Literals::alphaProgress);
		alpha->setTextVisible(false);
		alpha->setStyleSheet("QProgressBar::chunk { background-color: #595959 }");
		alpha->setFixedHeight(2);
		alpha->setMaximum(255);
		alpha->setValue(int(((glm::vec4*)value)->a * 255));
		layout->addWidget(alpha);
	}

	return widget;
}

QWidget* RendererInspector::drawColor3Field(uint index, const QString& name, const glm::vec3& value) {
	return drawColorField(index, name, VariantTypeColor3, &value);
}

QWidget* RendererInspector::drawColor4Field(uint index, const QString& name, const glm::vec4& value) {
	return drawColorField(index, name, VariantTypeColor4, &value);
}

QWidget* RendererInspector::drawVec3Field(uint index, const QString& name, const glm::vec3& value) {
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

QWidget* RendererInspector::drawVec4Field(uint index, const QString& name, const glm::vec4& value) {
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

void RendererInspector::onEditProperty() {
	QWidget* senderWidget = (QWidget*)sender();
	UserData* ud = ((UserData*)sender()->userData(Qt::UserRole));
	uint index = ud->index;
	QString name = ud->name;
	VariantType type = ud->type;

	switch (type) {
		case VariantTypeInt:
			dsp_cast<Renderer>(object_)->GetMaterial(index)->SetInt(name.toStdString(), ((QLineEdit*)sender())->text().toInt());
			break;
		case VariantTypeFloat:
			dsp_cast<Renderer>(object_)->GetMaterial(index)->SetFloat(name.toStdString(), ((QLineEdit*)sender())->text().toFloat());
			break;
		case VariantTypeVector3:
			break;
		case VariantTypeColor3:
			onSelectColor3(senderWidget, index, name);
			break;
		case VariantTypeColor4:
			onSelectColor4(senderWidget, index, name);
			break;
		case VariantTypeVector4:
			break;
		case VariantTypeTexture:
			onSelectTexture(senderWidget, index, name);
			break;
	}
}

void RendererInspector::onColorChanged(const QColor& color) {
	QColor selected = colorPicker_->currentColor();
	if (!selected.isValid()) {
		return;
	}

	UserData* data = (UserData*)colorPicker_->userData(Qt::UserRole);
	data->sender->setStyleSheet(QString::asprintf("border: 0; background-color: rgb(%d,%d,%d)",
		selected.red(), selected.green(), selected.blue()));

	Material material = dsp_cast<Renderer>(object_)->GetMaterial(data->index);
	if (data->type == VariantTypeColor4) {
		glm::vec4 newColor = Math::NormalizedColor(glm::ivec4(selected.red(), selected.green(), selected.blue(), selected.alpha()));
		material->SetColor4(data->name.toStdString(), newColor);

		QProgressBar* alpha = data->sender->parent()->findChild<QProgressBar*>(Literals::alphaProgress);
		alpha->setValue(selected.alpha());
	}
	else {
		glm::vec3 newColor = Math::NormalizedColor(glm::ivec3(selected.red(), selected.green(), selected.blue()));
		material->SetColor3(data->name.toStdString(), newColor);
	}
}


void RendererInspector::onSelectTexture(QWidget* widget, uint materialIndex, const QString& name) {
	QString path = QFileDialog::getOpenFileName(this, "Select texture", Resources::GetRootDirectory().c_str(), "*.jpg;;*.png");
	if (!path.isEmpty()) {
		Texture2D texture = NewTexture2D();
		QDir dir(Resources::GetRootDirectory().c_str());
		path = dir.relativeFilePath(path);
		texture->Load(path.toStdString());
		Material material = dsp_cast<Renderer>(object_)->GetMaterial(materialIndex);
		material->SetTexture(name.toStdString(), texture);
		((LabelTexture*)sender())->setTexture(texture);
	}
}

void RendererInspector::onSelectColor3(QWidget* widget, uint materialIndex, const QString& name) {
	colorPicker_->setOption(QColorDialog::ShowAlphaChannel, false);
	Material material = dsp_cast<Renderer>(object_)->GetMaterial(materialIndex);
	glm::ivec3 color = Math::IntColor(material->GetColor3(name.toStdString()));
	QColor old(color.r, color.g, color.b);

	colorPicker_->blockSignals(true);
	colorPicker_->setCurrentColor(old);
	colorPicker_->blockSignals(false);

	delete colorPicker_->userData(Qt::UserRole);
	colorPicker_->setUserData(Qt::UserRole, new UserData(materialIndex, name, VariantTypeColor3, widget));
	colorPicker_->exec();
}

void RendererInspector::onSelectColor4(QWidget* widget, uint materialIndex, const QString& name) {
	colorPicker_->setOption(QColorDialog::ShowAlphaChannel);
	Material material = dsp_cast<Renderer>(object_)->GetMaterial(materialIndex);
	glm::ivec4 color = Math::IntColor(material->GetColor4(name.toStdString()));
	QColor old(color.r, color.g, color.b, color.a);

	colorPicker_->blockSignals(true);
	colorPicker_->setCurrentColor(old);
	colorPicker_->blockSignals(false);

	delete colorPicker_->userData(Qt::UserRole);
	colorPicker_->setUserData(Qt::UserRole, new UserData(materialIndex, name, VariantTypeColor4, widget));
	colorPicker_->exec();
}
