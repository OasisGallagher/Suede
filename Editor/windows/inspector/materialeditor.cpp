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
	Texture2D texture = suede_dynamic_cast<Texture2D>(material->GetTexture(p->name));
	if (texture && GUI::ImageButton(p->name.c_str(), texture->GetNativePointer())) {
		QString path = QFileDialog::getOpenFileName(nullptr, "Select Texture", Resources::instance()->GetTextureDirectory().c_str(), "*.jpg;;*.png");
		if (!path.isEmpty()) {
			path = QDir(Resources::instance()->GetTextureDirectory().c_str()).relativeFilePath(path);
			commands_.push_back(new LoadTextureCommand(texture, path));
		}
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
