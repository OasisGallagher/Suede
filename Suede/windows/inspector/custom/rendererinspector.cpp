#include <QGroupBox>
#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QFileDialog>
#include <QProgressBar>

#include "tools/math2.h"
#include "tools/string.h"

#include "variables.h"
#include "resources.h"
#include "rendererinspector.h"

#include "widgets/fields/intfield.h"
#include "widgets/fields/vec3field.h"
#include "widgets/fields/vec4field.h"
#include "widgets/fields/rangefield.h"
#include "widgets/fields/colorfield.h"
#include "widgets/fields/floatfield.h"
#include "widgets/fields/texturefield.h"

#include "widgets/fields/filetreefield.h"

static const char* shaderRegex = ".*\\.shader";
static QString shaderDirectory = "resources/shaders/";

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
	Renderer renderer = suede_dynamic_cast<Renderer>(target_);
	QListWidget* materialList = new QListWidget(this);

	for (Material material : renderer->GetMaterials()) {
		materialList->addItem(material->GetName().c_str());
	}

	form_->setWidget(form_->rowCount(), QFormLayout::SpanningRole, materialList);
	resizeGeometryToFit(materialList);

	QGroupBox* materials = new QGroupBox("Materials", this);
	QVBoxLayout* materialsLayout = new QVBoxLayout(materials);

	for (uint materialIndex = 0; materialIndex < renderer->GetMaterialCount(); ++materialIndex) {
		drawMaterial(renderer, materialIndex, materialsLayout);
	}

	form_->setWidget(form_->rowCount(), QFormLayout::SpanningRole, materials);
}

RendererInspector::~RendererInspector() {
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
	Shader shader = Resources::instance()->FindShader(shaderPath.toStdString());
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
	IntField* field = new IntField(this);
	field->setValue(value);
	field->setProperty(USER_PROPERTY, QVariant::fromValue(UserProperty(materialIndex, name, VariantTypeInt)));
	connect(field, SIGNAL(valueChanged(int)), this, SLOT(onEditProperty()));
	return field;
}

QWidget* RendererInspector::drawFloatField(uint materialIndex, const QString& name, float value) {
	FloatField* field = new FloatField(this);
	field->setValue(value);
	field->setProperty(USER_PROPERTY, QVariant::fromValue(UserProperty(materialIndex, name, VariantTypeFloat)));
	connect(field, SIGNAL(valueChanged(float)), this, SLOT(onEditProperty()));
	return field;
}

QWidget* RendererInspector::drawTextureField(uint materialIndex, const QString& name, Texture value) {
	TextureField* field = new TextureField(this);
	field->setTexture(value);

	field->setData(QVariant::fromValue(UserProperty(materialIndex, name, VariantTypeTexture)));
	connect(field, SIGNAL(currentTextureChanged(Texture)), this, SLOT(onCurrentTextureChanged(Texture)));
	return field;
}

QWidget* RendererInspector::drawColor3Field(uint materialIndex, const QString& name, const glm::vec3& value) {
	return drawColorField(materialIndex, name, VariantTypeColor3, &value);
}

QWidget* RendererInspector::drawColor4Field(uint materialIndex, const QString& name, const glm::vec4& value) {
	return drawColorField(materialIndex, name, VariantTypeColor4, &value);
}

QWidget* RendererInspector::drawColorField(uint materialIndex, const QString& name, VariantType type, const void* value) {
	ColorField* field = new ColorField(this);
	field->setProperty("UserProperty", QVariant::fromValue(UserProperty(materialIndex, name, type)));

	if (type == VariantTypeColor3) {
		field->setValue(*(const glm::vec3*)value);
	}
	else {
		field->setValue(*(const glm::vec4*)value);
	}

	connect(field, SIGNAL(valueChanged(const QColor&)), this, SLOT(onCurrentColorChanged(const QColor&)));

	return field;
}

QWidget* RendererInspector::drawVec3Field(uint materialIndex, const QString& name, const glm::vec3& value) {
	Vec3Field* field = new Vec3Field(this);
	field->setValue(value);
	connect(field, SIGNAL(valueChanged(const glm::vec3&)), this, SLOT(onEditProperty()));

	QVariant variant = QVariant::fromValue(UserProperty(materialIndex, name, VariantTypeVector3));
	field->setProperty(USER_PROPERTY, variant);

	return field;
}

QWidget* RendererInspector::drawVec4Field(uint materialIndex, const QString& name, const glm::vec4& value) {
	Vec4Field* field = new Vec4Field(this);
	field->setValue(value);
	connect(field, SIGNAL(valueChanged(const glm::vec4&)), this, SLOT(onEditProperty()));

	QVariant variant = QVariant::fromValue(UserProperty(materialIndex, name, VariantTypeVector4));
	field->setProperty(USER_PROPERTY, variant);

	return field;
}

void RendererInspector::onEditProperty() {
	QWidget* senderWidget = (QWidget*)sender();
	UserProperty prop = sender()->property(USER_PROPERTY).value<UserProperty>();

	switch (prop.type) {
		case VariantTypeInt:
			suede_dynamic_cast<Renderer>(target_)->GetMaterial(prop.materialIndex)->SetInt(prop.name.toStdString(), ((IntField*)sender())->value());
			break;
		case VariantTypeFloat:
			suede_dynamic_cast<Renderer>(target_)->GetMaterial(prop.materialIndex)->SetFloat(prop.name.toStdString(), ((FloatField*)sender())->value());
			break;
		case VariantTypeVector3:
			break;
		case VariantTypeVector4:
			break;
	}
}

void RendererInspector::onCurrentColorChanged(const QColor& color) {
	ColorField* field = (ColorField*)sender();
	UserProperty prop = field->property("UserProperty").value<UserProperty>();

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
