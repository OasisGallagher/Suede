#include "materialeditor.h"

#include <QFileDialog>

#include "resources.h"
#include "../widgets/dialogs/shaderselector.h"

void MaterialEditor::draw(Material* material, QWidget* parent) {
	constantDraw(material, parent).apply(material);
}

MaterialEditor::Modification MaterialEditor::constantDraw(Material* material, QWidget* parent) {
	Modification modification;
	std::string materialName = material->GetName();
	if (GUI::CollapsingHeader(materialName.c_str())) {
		GUI::BeginScope(materialName.c_str());

		GUI::Indent();
		modification.shader = drawShaderSelector(material, parent);
		drawProperties(modification.properties, material);
		GUI::Unindent();

		GUI::EndScope();
	}

	return modification;
}

Shader* MaterialEditor::drawShaderSelector(Material* material, QWidget* parent) {
	std::string shaderName = material->GetShader()->GetName();
	if (GUI::Button(shaderName.c_str())) {
		ShaderSelector selector(parent);
		std::string fullShaderPath = selector.select(shaderName.c_str()).toStdString();
		if (!fullShaderPath.empty()) {
			std::string shaderName = fullShaderPath.substr(strlen(Resources::shaderDirectory));
			shaderName = shaderName.substr(0, shaderName.length() - strlen(".shader"));
			return Shader::Find(shaderName);
		}
	}

	return nullptr;
}

void MaterialEditor::drawProperties(QList<Property>& changed, Material* material) {
	for (const Property* p : material->GetExplicitProperties()) {
		switch (p->value.GetType()) {
			case VariantType::Bool:
				drawBoolProperty(changed, material, p);
				break;
			case VariantType::RangedInt:
				drawRangedIntProperty(changed, material, p);
				break;
			case VariantType::RangedFloat:
				drawRangedFloatProperty(changed, material, p);
				break;
			case VariantType::Float:
				drawFloatProperty(changed, material, p);
				break;
			case VariantType::Vector3:
				drawVector3Property(changed, material, p);
				break;
			case VariantType::Vector4:
				drawVector4Property(changed, material, p);
				break;
			case VariantType::Color:
				drawColorProperty(changed, material, p);
				break;
			case VariantType::Texture:
				drawTextureProperty(changed, material, p);
				break;
		}
	}
}

void MaterialEditor::drawTextureProperty(QList<Property>& changed, Material* material, const Property* p) {
	Texture* texture = material->GetTexture(p->name);
	Texture2D* texture2D = (Texture2D*)texture;

	// editable texture.
	if (texture2D) {
		if (GUI::ImageButton(p->name.c_str(), texture2D->GetNativePointer())) {
			QString path = QFileDialog::getOpenFileName(nullptr, "Select Texture", Resources::textureDirectory, "*.jpg;*.png;*.dds");
			if (!path.isEmpty()) {
				path = QDir(Resources::textureDirectory).relativeFilePath(path);
				ref_ptr<Texture2D> newTexture = new Texture2D();
				newTexture->Load(path.toStdString());
				changed.push_back({ p->name, newTexture.get() });
			}
		}
	}
	else if (texture) {
		GUI::Image(p->name.c_str(), texture->GetNativePointer());
	}
}

void MaterialEditor::drawBoolProperty(QList<Property>& changed, Material* material, const Property* p) {
	bool value = material->GetBool(p->name);
	if (GUI::Toggle(p->name.c_str(), value)) {
		changed.push_back({ p->name, value });
	}
}

void MaterialEditor::drawRangedIntProperty(QList<Property>& changed, Material* material, const Property* p) {
	iranged r = material->GetRangedInt(p->name);
	int value = r.value();
	if (GUI::IntSlider(p->name.c_str(), value, r.min(), r.max())) {
		changed.push_back({ p->name, iranged(value, r.min(), r.max()) });
	}
}

void MaterialEditor::drawRangedFloatProperty(QList<Property>& changed, Material* material, const Property* p) {
	franged r = material->GetRangedFloat(p->name);
	float value = r.value();
	if (GUI::Slider(p->name.c_str(), value, r.min(), r.max())) {
		changed.push_back({ p->name, franged(value, r.min(), r.max()) });
	}
}

void MaterialEditor::drawColorProperty(QList<Property>& changed, Material* material, const Property* p) {
	Color value = material->GetColor(p->name);
	if (GUI::ColorField(p->name.c_str(), value)) {
		changed.push_back({ p->name, value });
	}
}

void MaterialEditor::drawFloatProperty(QList<Property>& changed, Material* material, const Property* p) {
	float value = material->GetFloat(p->name);
	if (GUI::FloatField(p->name.c_str(), value)) {
		changed.push_back({ p->name, value });
	}
}

void MaterialEditor::drawVector3Property(QList<Property>& changed, Material* material, const Property* p) {
	Vector3 value = material->GetVector3(p->name);
	if (GUI::Float3Field(p->name.c_str(), value)) {
		changed.push_back({ p->name, value });
	}
}

void MaterialEditor::drawVector4Property(QList<Property>& changed, Material* material, const Property* p) {
	Vector4 value = material->GetVector4(p->name);
	if (GUI::Float4Field(p->name.c_str(), value)) {
		changed.push_back({ p->name, value });
	}
}

bool MaterialEditor::Modification::empty() const {
	return shader == nullptr && properties.empty();
}

void MaterialEditor::Modification::apply(Material* material) {
	if (shader != nullptr) { material->SetShader(shader); }
	for (const Property& p : properties) {
		material->SetVariant(p.name, p.value);
	}
}
