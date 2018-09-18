#include <qstring.h>

#include "gui.h"
#include "renderer.h"
#include "os/filesystem.h"
#include "custominspector.h"

class MeshRendererInspector : public CustomInspector {
public:
	virtual void onGui(Component component);

private:
	class LoadTextureCommand : public MainContextCommand {
	public:
		LoadTextureCommand(Texture2D tex, const QString& p) : texture(tex), path(p) {}

	public:
		virtual void run() { texture->Create(path.toStdString()); }

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
	void drawColor(Material material, const Property* p);
	void drawVector3(Material material, const Property* p);
	void drawVector4(Material material, const Property* p);
};
