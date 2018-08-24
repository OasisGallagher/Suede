#pragma once
#include <QDockWidget>

#include <glm/glm.hpp>

#include "light.h"
#include "camera.h"
#include "projector.h"
#include "../winbase.h"

class CustomInspector;

class Inspector : public QDockWidget, public WinSingleton<Inspector> {
	Q_OBJECT

public:
	Inspector(QWidget* parent);
	~Inspector();

public:
	virtual void init(Ui::Suede* ui);
	virtual void awake();
	virtual void tick();


private slots:
	void onSelectionChanged(const QList<Entity>& selected, const QList<Entity>& deselected);

private:
	void onGui();
	void drawGui();

	void addInspector(CustomInspector* inspector);

	void drawBasics();
	void drawTags();
	void drawTransform();
	void drawComponents();

	void drawLight(Light light);
	void drawCamera(Camera camera);
	void drawProjector(Projector projector);

	void drawMesh(Mesh mesh);
	void drawRenderer(Renderer renderer);
	void drawMaterial(Material material);

	void drawTexture(Material material, const Property* p);

	void drawColor3(Material material, const Property* p);
	void drawColor4(Material material, const Property* p);

	void drawSingle(Material material, const Property* p);
	void drawSingle3(Material material, const Property* p);
	void drawSingle4(Material material, const Property* p);

private:
	QGLWidget* view_;

	Entity target_;
};
