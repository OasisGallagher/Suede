#include "materialeditor.h"

#include <QFileDialog>

#include "resources.h"

QVector<MainContextCommand*> MaterialEditor::commands_;

class LoadTextureCommand : public MainContextCommand {
public:
	LoadTextureCommand(Texture2D tex, const QString& p) : texture(tex), path(p) {}

public:
	virtual void run() { texture->Create(path.toStdString()); }

private:
	QString path;
	Texture2D texture;
};

void MaterialEditor::draw(Material material) {
	if (GUI::CollapsingHeader(material->GetName().c_str())) {
		GUI::Indent();
		drawShaderSelector(material);
		drawProperties(material);
		GUI::Unindent();
	}
}

void MaterialEditor::drawShaderSelector(Material material) {
	// SUEDE TODO: shader selector.
}

void MaterialEditor::drawProperties(Material material) {
	std::vector<const Property*> properties;
	material->GetProperties(properties);

	for (const Property* p : properties) {
		switch (p->value.GetType()) {
			case VariantType::Bool:
				drawBoolProperty(material, p);
				break;
			case VariantType::RangedInt:
				drawRangedIntProperty(material, p);
				break;
			case VariantType::RangedFloat:
				drawRangedFloatProperty(material, p);
				break;
			case VariantType::Float:
				drawFloatProperty(material, p);
				break;
			case VariantType::Vector3:
				drawVector3Property(material, p);
				break;
			case VariantType::Vector4:
				drawVector4Property(material, p);
				break;
			case VariantType::Color:
				drawColorProperty(material, p);
				break;
			case VariantType::Texture:
				drawTextureProperty(material, p);
				break;
		}
	}
}

void MaterialEditor::runMainContextCommands() {
	for (MainContextCommand* cmd : commands_) {
		cmd->run();
		delete cmd;
	}

	commands_.clear();
}

void MaterialEditor::drawTextureProperty(Material material, const Property* p) {
	Texture texture = material->GetTexture(p->name);
	Texture2D texture2D = suede_dynamic_cast<Texture2D>(texture);

	// editable texture.
	if (texture2D) {
		drawTexture2DSelector(p, texture2D);
	}
	else if (texture) {
		GUI::Image(p->name.c_str(), texture->GetNativePointer());
	}
}

void MaterialEditor::drawTexture2DSelector(const Property* p, Texture2D texture2D) {
	if (GUI::ImageButton(p->name.c_str(), texture2D->GetNativePointer())) {
		QString path = QFileDialog::getOpenFileName(nullptr, "Select Texture", Resources::instance()->GetTextureDirectory().c_str(), "*.jpg;;*.png");
		if (!path.isEmpty()) {
			path = QDir(Resources::instance()->GetTextureDirectory().c_str()).relativeFilePath(path);
			commands_.push_back(new LoadTextureCommand(texture2D, path));
		}
	}
}

void MaterialEditor::drawBoolProperty(Material material, const Property* p) {
	bool value = material->GetBool(p->name);
	if (GUI::Toggle(p->name.c_str(), value)) {
		material->SetBool(p->name, value);
	}
}

void MaterialEditor::drawRangedIntProperty(Material material, const Property* p) {
	iranged r = material->GetRangedInt(p->name);
	int i = r.value();
	if (GUI::IntSlider(p->name.c_str(), i, r.min(), r.max())) {
		material->SetInt(p->name, i);
	}
}

void MaterialEditor::drawRangedFloatProperty(Material material, const Property* p) {
	franged r = material->GetRangedFloat(p->name);
	float f = r.value();
	if (GUI::Slider(p->name.c_str(), f, r.min(), r.max())) {
		material->SetFloat(p->name, f);
	}
}

void MaterialEditor::drawColorProperty(Material material, const Property* p) {
	Color value = material->GetColor(p->name);
	if (GUI::ColorField(p->name.c_str(), value)) {
		material->SetColor(p->name, value);
	}
}

void MaterialEditor::drawFloatProperty(Material material, const Property* p) {
	float value = material->GetFloat(p->name);
	if (GUI::FloatField(p->name.c_str(), value)) {
		material->SetFloat(p->name, value);
	}
}

void MaterialEditor::drawVector3Property(Material material, const Property* p) {
	glm::vec3 value = material->GetVector3(p->name);
	if (GUI::Float3Field(p->name.c_str(), value)) {
		material->SetVector3(p->name, value);
	}
}

void MaterialEditor::drawVector4Property(Material material, const Property* p) {
	glm::vec4 value = material->GetVector4(p->name);
	if (GUI::Float4Field(p->name.c_str(), value)) {
		material->SetVector4(p->name, value);
	}
}
