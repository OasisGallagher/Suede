#include "meshrendererinspector.h"

#include <QFileDialog>

#include "resources.h"


void MeshRendererInspector::onGui() {
	for (Material material : target_->GetMaterials()) {
		drawMaterial(material);
	}
}

void MeshRendererInspector::drawMaterial(Material material) {
	GUI::Separator();
	if (GUI::CollapsingHeader(material->GetName().c_str())) {
		GUI::Indent();
		drawMaterialShader(material);
		drawMaterialProperties(material);
		GUI::Unindent();
	}
}

void MeshRendererInspector::drawMaterialShader(Material material) {
	GUI::LabelField("Shader", material->GetShader()->GetName().c_str());
	/*FileTree tree;
	std::string shaderName = material->GetShader()->GetName();
	std::string directory = "resources/shaders/", regex = ".*\\.shader";

	if (FileSystem::ListFileTree(tree, directory, regex)) {
	if (GUI::BeginMenu(shaderName.c_str())) {
	drawMaterialShaderMenu(tree.GetRoot());
	GUI::EndMenu();
	}
	}*/
}

void MeshRendererInspector::drawMaterialShaderMenu(FileEntry* entry) {
	for (uint i = 0; i < entry->GetChildCount(); ++i) {
		FileEntry* child = entry->GetChildAt(i);
		std::string name = child->GetName();
		if (child->IsDirectory()) {
			if (GUI::BeginMenu(name.substr(name.length() - 1).c_str())) {
				drawMaterialShaderMenu(child);
				GUI::EndMenu();
			}
		}
		else {
			name = FileSystem::GetFileNameWithoutExtension(name);
			if (GUI::MenuItem(name.c_str(), false)) {

			}
		}
	}
}

void MeshRendererInspector::drawMaterialProperties(Material material) {
	std::vector<const Property*> properties;
	material->GetProperties(properties);

	for (const Property* p : properties) {
		switch (p->value.GetType()) {
			case VariantType::Float:
				drawFloat(material, p);
				break;
			case VariantType::Vector3:
				drawVector3(material, p);
				break;
			case VariantType::Vector4:
				drawVector4(material, p);
				break;
			case VariantType::Color3:
				drawColor3(material, p);
				break;
			case VariantType::Color4:
				drawColor4(material, p);
				break;
			case VariantType::Texture:
				drawTexture(material, p);
				break;
		}
	}
}

void MeshRendererInspector::drawTexture(Material material, const Property* p) {
	Texture2D texture = suede_dynamic_cast<Texture2D>(material->GetTexture(p->name));
	if (texture && GUI::ImageButton(p->name.c_str(), texture->GetNativePointer())) {
		QString path = QFileDialog::getOpenFileName(nullptr, "Select Texture", Resources::instance()->GetTextureDirectory().c_str(), "*.jpg;;*.png");
		if (!path.isEmpty()) {
			path = QDir(Resources::instance()->GetTextureDirectory().c_str()).relativeFilePath(path);
			addMainContextCommand(new LoadTextureCommand(texture, path));
		}
	}
}

void MeshRendererInspector::drawColor3(Material material, const Property* p) {
	glm::vec3 value = material->GetColor3(p->name);
	if (GUI::Color3Field(p->name.c_str(), value)) {
		material->SetColor3(p->name, value);
	}
}

void MeshRendererInspector::drawColor4(Material material, const Property* p) {
	glm::vec4 value = material->GetColor4(p->name);
	if (GUI::Color4Field(p->name.c_str(), value)) {
		material->SetColor4(p->name, value);
	}
}

void MeshRendererInspector::drawFloat(Material material, const Property* p) {
	float value = material->GetFloat(p->name);
	if (GUI::FloatField(p->name.c_str(), value)) {
		material->SetFloat(p->name, value);
	}
}

void MeshRendererInspector::drawVector3(Material material, const Property* p) {
	glm::vec3 value = material->GetVector3(p->name);
	if (GUI::Float3Field(p->name.c_str(), value)) {
		material->SetVector3(p->name, value);
	}
}

void MeshRendererInspector::drawVector4(Material material, const Property* p) {
	glm::vec4 value = material->GetVector4(p->name);
	if (GUI::Float4Field(p->name.c_str(), value)) {
		material->SetVector4(p->name, value);
	}
}
