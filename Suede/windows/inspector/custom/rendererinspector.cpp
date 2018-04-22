#include <QGroupBox>
#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QFileDialog>
#include <QProgressBar>

#include "tools/math2.h"
#include "debug/profiler.h"

#include "variables.h"
#include "resources.h"
#include "rendererinspector.h"
#include "windows/controls/colorpicker.h"
#include "windows/controls/labeltexture.h"

Sample* render_inspector = Profiler::CreateSample();

static void begin_render_inspector() {
	render_inspector->Restart();
}

static void end_render_inspector(const char* text) {
	render_inspector->Stop();
	Debug::Log((std::string(text) + " %.2f").c_str(), render_inspector->GetElapsedSeconds());
}

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
	begin_render_inspector();
	Renderer renderer = suede_dynamic_cast<Renderer>(target_);
	QListWidget* materialList = new QListWidget(this);
	end_render_inspector("createMaterialList");

	begin_render_inspector();
	for (int i = 0; i < renderer->GetMaterialCount(); ++i) {
		Material material = renderer->GetMaterial(i);
		materialList->addItem(material->GetName().c_str());
	}
	end_render_inspector("materialList");

	begin_render_inspector();
	form_->setWidget(form_->rowCount(), QFormLayout::SpanningRole, materialList);
	resizeGeometryToFit(materialList);
	end_render_inspector("resizeGeometry");

	begin_render_inspector();
	QStringList list;
	const std::vector<ShaderResource>& shaders = Resources::GetShaderResources();
	for (int i = 0; i < shaders.size(); ++i) {
		list << shaders[i].name.c_str();
	}
	end_render_inspector("appendShaders");

	begin_render_inspector();
	QGroupBox* materials = new QGroupBox("Materials", this);
	QVBoxLayout* materialsLayout = new QVBoxLayout(materials);

	for (uint materialIndex = 0; materialIndex < renderer->GetMaterialCount(); ++materialIndex) {
		drawMaterial(renderer, materialIndex, list, materialsLayout);
	}
	end_render_inspector("drawMaterial");

	begin_render_inspector();
	form_->setWidget(form_->rowCount(), QFormLayout::SpanningRole, materials);
	connect(ColorPicker::get(), SIGNAL(currentColorChanged(const QColor&)), this, SLOT(onColorPicked(const QColor&)));
	end_render_inspector("misc");
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
	QLineEdit* line = new QLineEdit(QString::number(value));
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

	QLineEdit* x = new QLineEdit(QString::number(value.x), widget);
	QLineEdit* y = new QLineEdit(QString::number(value.y), widget);
	QLineEdit* z = new QLineEdit(QString::number(value.z), widget);

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

	QLineEdit* x = new QLineEdit(QString::number(value.x), widget);
	QLineEdit* y = new QLineEdit(QString::number(value.y), widget);
	QLineEdit* z = new QLineEdit(QString::number(value.z), widget);
	QLineEdit* w = new QLineEdit(QString::number(value.w), widget);

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
			suede_dynamic_cast<Renderer>(target_)->GetMaterial(index)->SetInt(name.toStdString(), ((QLineEdit*)sender())->text().toInt());
			break;
		case VariantTypeFloat:
			suede_dynamic_cast<Renderer>(target_)->GetMaterial(index)->SetFloat(name.toStdString(), ((QLineEdit*)sender())->text().toFloat());
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

void RendererInspector::onColorPicked(const QColor& color) {
	QColor selected = ColorPicker::get()->currentColor();
	if (!selected.isValid()) {
		return;
	}

	UserData* data = (UserData*)ColorPicker::get()->userData(Qt::UserRole);
	data->sender->setStyleSheet(QString::asprintf("border: 0; background-color: rgb(%d,%d,%d)",
		selected.red(), selected.green(), selected.blue()));

	Material material = suede_dynamic_cast<Renderer>(target_)->GetMaterial(data->index);
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
		Material material = suede_dynamic_cast<Renderer>(target_)->GetMaterial(materialIndex);
		material->SetTexture(name.toStdString(), texture);
		((LabelTexture*)sender())->setTexture(texture);
	}
}

void RendererInspector::onSelectColor3(QWidget* widget, uint materialIndex, const QString& name) {
	ColorPicker::get()->setOption(QColorDialog::ShowAlphaChannel, false);
	Material material = suede_dynamic_cast<Renderer>(target_)->GetMaterial(materialIndex);
	glm::ivec3 color = Math::IntColor(material->GetColor3(name.toStdString()));
	QColor old(color.r, color.g, color.b);

	ColorPicker::get()->blockSignals(true);
	ColorPicker::get()->setCurrentColor(old);
	ColorPicker::get()->blockSignals(false);

	delete ColorPicker::get()->userData(Qt::UserRole);
	ColorPicker::get()->setUserData(Qt::UserRole, new UserData(materialIndex, name, VariantTypeColor3, widget));
	ColorPicker::get()->exec();
}

void RendererInspector::onSelectColor4(QWidget* widget, uint materialIndex, const QString& name) {
	ColorPicker::get()->setOption(QColorDialog::ShowAlphaChannel);
	Material material = suede_dynamic_cast<Renderer>(target_)->GetMaterial(materialIndex);
	glm::ivec4 color = Math::IntColor(material->GetColor4(name.toStdString()));
	QColor old(color.r, color.g, color.b, color.a);

	ColorPicker::get()->blockSignals(true);
	ColorPicker::get()->setCurrentColor(old);
	ColorPicker::get()->blockSignals(false);

	delete ColorPicker::get()->userData(Qt::UserRole);
	ColorPicker::get()->setUserData(Qt::UserRole, new UserData(materialIndex, name, VariantTypeColor4, widget));
	ColorPicker::get()->exec();
}
