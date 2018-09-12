#include "gui.h"
#include "renderer.h"
#include "os/filesystem.h"
#include "custominspector.h"

class MeshRendererInspector : public CustomInspectorT<Renderer> {
public:
	virtual void onGui();

private:
	class LoadTextureCommand : public MainContextCommand {
	public:
		LoadTextureCommand(Texture2D tex, const QString& p) : texture(tex), path(p) {}

	public:
		virtual void Run() { texture->Create(path.toStdString()); }

	private:
		QString path;
		Texture2D texture;
	};

private:
	void drawMaterial(Material material);
	void drawMaterialShader(Material material);
	void drawMaterialShaderMenu(FileEntry* entry);
	void drawMaterialProperties(Material material);

	void drawFloat(Material material, const Property* p);
	void drawTexture(Material material, const Property* p);
	void drawColor3(Material material, const Property* p);
	void drawColor4(Material material, const Property* p);
	void drawVector3(Material material, const Property* p);
	void drawVector4(Material material, const Property* p);
};
