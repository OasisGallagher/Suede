#include <QGroupBox>
#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QFileDialog>
#include <QProgressBar>

#include "tools/math2.h"
#include "tools/string.h"
#include "debug/profiler.h"

#include "variables.h"
#include "resources.h"
#include "rendererinspector.h"

#include "windows/controls/colorfield.h"
#include "windows/controls/texturefield.h"

#include "windows/controls/filetreefield.h"

static const char* shaderRegex = ".*\\.shader";
static QString shaderDirectory = "resources/shaders/";
static Sample* render_inspector = Profiler::CreateSample();

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

#define USER_PROPERTY	"USER_PROPERTY"

struct UserProperty {
	UserProperty() : UserProperty(0, "", VariantTypeNone) {}
	UserProperty(uint materialIndex, const QString& name, VariantType type, QWidget* sender = nullptr) {
		this->materialIndex = materialIndex;
		this->name = name;
		this->type = type;
		this->sender = sender;
	}

	QString name;
	QWidget* sender;
	VariantType type;
	uint materialIndex;
};

Q_DECLARE_METATYPE(UserProperty);

struct MaterialProperty {
	MaterialProperty() : MaterialProperty(nullptr, nullptr, 0) {}
	MaterialProperty(QLayout* layout, QGroupBox* g, int materialIndex) {
		this->groupBox = g;
		this->layout = layout;
		this->materialIndex = materialIndex;
	}

	QGroupBox* groupBox;
	QLayout* layout;
	uint materialIndex;
};

Q_DECLARE_METATYPE(MaterialProperty);

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
	QGroupBox* materials = new QGroupBox("Materials", this);
	QVBoxLayout* materialsLayout = new QVBoxLayout(materials);

	for (uint materialIndex = 0; materialIndex < renderer->GetMaterialCount(); ++materialIndex) {
		drawMaterial(renderer, materialIndex, materialsLayout);
	}
	end_render_inspector("createMaterialList");

	begin_render_inspector();
	form_->setWidget(form_->rowCount(), QFormLayout::SpanningRole, materials);
	end_render_inspector("appendMaterialList");
}

void RendererInspector::drawMaterial(Renderer renderer, uint materialIndex, QLayout* materialsLayout) {
	QGroupBox* g = createMaterialBox(renderer, materialIndex, materialsLayout);
	materialsLayout->addWidget(g);
}

QGroupBox* RendererInspector::createMaterialBox(Renderer renderer, uint materialIndex, QLayout* materialsLayout) {
	Material material = renderer->GetMaterial(materialIndex);
	Shader shader = material->GetShader();

	QGroupBox* g = new QGroupBox(material->GetName().c_str());

	QFormLayout* form = new QFormLayout(g);
	g->setLayout(form);

	FileTreeField* field = new FileTreeField(g);
	field->setProperty(USER_PROPERTY, QVariant::fromValue(MaterialProperty(materialsLayout, g, materialIndex)));
	field->setDirectory(shaderDirectory, material->GetShader()->GetName().c_str(), shaderRegex);
	connect(field, SIGNAL(selectionChanged(const QString&)), this, SLOT(onShaderSelectionChanged(const QString&)));

	form->addRow(formatRowName("Shader"), field);

	QWidgetList widgets;
	drawMaterialProperties(widgets, material, materialIndex);

	foreach(QWidget* w, widgets) {
		form->addRow(formatRowName(w->objectName()), w);
		w->setParent(g);
	}

	return g;
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
			if (String::StartsWith(p->name, VARIABLE_PREFIX)) {
				ptr += strlen(VARIABLE_PREFIX);
			}

			widget->setObjectName(ptr);
			widgets.push_back(widget);
		}
	}
}

bool RendererInspector::updateMaterial(uint materialIndex, const QString& shaderPath) {
	Shader shader = Resources::FindShader(shaderPath.toStdString());
	if (!shader) {
		return false;
	}

	Material material = NewMaterial();
	material->SetShader(shader);

	Renderer renderer = suede_dynamic_cast<Renderer>(target_);
	Material oldMaterial = renderer->GetMaterial(materialIndex);

	std::vector<const Property*> properties;
	oldMaterial->GetProperties(properties);
	for (uint i = 0; i < properties.size(); ++i) {
		material->SetVariant(properties[i]->name, properties[i]->value);
	}

	renderer->SetMaterial(materialIndex, material);

	return true;
}

bool RendererInspector::isPropertyVisible(const QString& name) {
	return !name.startsWith(VARIABLE_PREFIX)
		|| name == Variables::MainTexture
		|| name == Variables::BumpTexture
		|| name == Variables::SpecularTexture
		|| name == Variables::EmissiveTexture
		|| name == Variables::LightmapTexture
		|| name == Variables::Gloss
		|| name == Variables::MainColor
		|| name == Variables::SpecularColor
		|| name == Variables::EmissiveColor;
}

QWidget* RendererInspector::drawIntField(uint materialIndex, const QString& name, int value) {
	QLineEdit* line = new QLineEdit(QString::number(value));
	line->setValidator(new QIntValidator(line));

	line->setProperty(USER_PROPERTY, QVariant::fromValue(UserProperty(materialIndex, name, VariantTypeInt)));
	connect(line, SIGNAL(editingFinished()), this, SLOT(onEditProperty()));

	return line;
}

QWidget* RendererInspector::drawFloatField(uint materialIndex, const QString& name, float value) {
	QLineEdit* line = new QLineEdit(QString::number(value));
	line->setValidator(new QDoubleValidator(line));

	line->setProperty(USER_PROPERTY, QVariant::fromValue(UserProperty(materialIndex, name, VariantTypeFloat)));
	connect(line, SIGNAL(editingFinished()), this, SLOT(onEditProperty()));

	return line;
}

QWidget* RendererInspector::drawTextureField(uint materialIndex, const QString& name, Texture value) {
	TextureField* field = new TextureField(this);
	field->setTexture(value);

	field->setData(QVariant::fromValue(UserProperty(materialIndex, name, VariantTypeTexture)));
	connect(field, SIGNAL(currentTextureChanged(Texture)), this, SLOT(onCurrentTextureChanged(Texture)));
	return field;
}

QWidget* RendererInspector::drawColorField(uint materialIndex, const QString& name, VariantType type, const void* value) {
	ColorField* field = new ColorField(this);
	field->setData(QVariant::fromValue(UserProperty(materialIndex, name, type)));

	if (type == VariantTypeColor3) {
		field->setColor(*(const glm::vec3*)value);
	}
	else {
		field->setColor(*(const glm::vec4*)value);
	}
	connect(field, SIGNAL(currentColorChanged(const QColor&)), this, SLOT(onCurrentColorChanged(const QColor&)));

	return field;
}

QWidget* RendererInspector::drawColor3Field(uint materialIndex, const QString& name, const glm::vec3& value) {
	return drawColorField(materialIndex, name, VariantTypeColor3, &value);
}

QWidget* RendererInspector::drawColor4Field(uint materialIndex, const QString& name, const glm::vec4& value) {
	return drawColorField(materialIndex, name, VariantTypeColor4, &value);
}

QWidget* RendererInspector::drawVec3Field(uint materialIndex, const QString& name, const glm::vec3& value) {
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

	QVariant variant = QVariant::fromValue(UserProperty(materialIndex, name, VariantTypeVector3));
	x->setProperty(USER_PROPERTY, variant);
	connect(x, SIGNAL(editingFinished()), this, SLOT(onEditProperty()));

	y->setProperty(USER_PROPERTY, variant);
	connect(y, SIGNAL(editingFinished()), this, SLOT(onEditProperty()));

	z->setProperty(USER_PROPERTY, variant);
	connect(z, SIGNAL(editingFinished()), this, SLOT(onEditProperty()));

	layout->addWidget(x);
	layout->addWidget(y);
	layout->addWidget(z);

	return widget;
}

QWidget* RendererInspector::drawVec4Field(uint materialIndex, const QString& name, const glm::vec4& value) {
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

	QVariant variant = QVariant::fromValue(UserProperty(materialIndex, name, VariantTypeVector4));
	x->setProperty(USER_PROPERTY, variant);
	connect(x, SIGNAL(editingFinished()), this, SLOT(onEditProperty()));

	y->setProperty(USER_PROPERTY, variant);
	connect(y, SIGNAL(editingFinished()), this, SLOT(onEditProperty()));

	z->setProperty(USER_PROPERTY, variant);
	connect(z, SIGNAL(editingFinished()), this, SLOT(onEditProperty()));

	w->setProperty(USER_PROPERTY, variant);
	connect(w, SIGNAL(editingFinished()), this, SLOT(onEditProperty()));

	layout->addWidget(x);
	layout->addWidget(y);
	layout->addWidget(z);
	layout->addWidget(w);

	return widget;
}

void RendererInspector::onEditProperty() {
	QWidget* senderWidget = (QWidget*)sender();
	UserProperty prop = sender()->property(USER_PROPERTY).value<UserProperty>();

	switch (prop.type) {
		case VariantTypeInt:
			suede_dynamic_cast<Renderer>(target_)->GetMaterial(prop.materialIndex)->SetInt(prop.name.toStdString(), ((QLineEdit*)sender())->text().toInt());
			break;
		case VariantTypeFloat:
			suede_dynamic_cast<Renderer>(target_)->GetMaterial(prop.materialIndex)->SetFloat(prop.name.toStdString(), ((QLineEdit*)sender())->text().toFloat());
			break;
		case VariantTypeVector3:
			break;
		case VariantTypeVector4:
			break;
	}
}

void RendererInspector::onCurrentColorChanged(const QColor& color) {
	ColorField* field = (ColorField*)sender();
	UserProperty prop = field->data().value<UserProperty>();

	Material material = suede_dynamic_cast<Renderer>(target_)->GetMaterial(prop.materialIndex);
	if (prop.type == VariantTypeColor4) {
		glm::vec4 newColor = Math::NormalizedColor(glm::ivec4(color.red(), color.green(), color.blue(), color.alpha()));
		material->SetColor4(prop.name.toStdString(), newColor);
	}
	else {
		glm::vec3 newColor = Math::NormalizedColor(glm::ivec3(color.red(), color.green(), color.blue()));
		material->SetColor3(prop.name.toStdString(), newColor);
	}
}

void RendererInspector::onShaderSelectionChanged(const QString& path) {
	uint length = shaderDirectory.length();
	MaterialProperty p = ((QComboBox*)sender())->property(USER_PROPERTY).value<MaterialProperty>();

	if (updateMaterial(p.materialIndex, path.mid(length, path.lastIndexOf('.') - length))) {
		Renderer renderer = suede_dynamic_cast<Renderer>(target_);
		QGroupBox* g = createMaterialBox(renderer, p.materialIndex, p.layout);
		p.layout->replaceWidget(p.groupBox, g);
		p.groupBox->deleteLater();
	}
}

void RendererInspector::onCurrentTextureChanged(Texture texture) {
	TextureField* field = (TextureField*)sender();
	UserProperty prop = field->data().value<UserProperty>();
	Material material = suede_dynamic_cast<Renderer>(target_)->GetMaterial(prop.materialIndex);
	material->SetTexture(prop.name.toStdString(), texture);
}
