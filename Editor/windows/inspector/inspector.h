#pragma once
#include <QDockWidget>

#include <glm/glm.hpp>

#include "light.h"
#include "camera.h"
#include "texture.h"
#include "projector.h"
#include "../winbase.h"

class CustomInspector;

class FileEntry;
class Inspector : public QDockWidget, public WinSingleton<Inspector> {
	Q_OBJECT

public:
	Inspector(QWidget* parent);
	~Inspector();

public:
	virtual void init(Ui::Editor* ui);
	virtual void awake();
	virtual void tick();


private slots:
	void onSelectionChanged(const QList<Entity>& selected, const QList<Entity>& deselected);

private:
	void onGui();

	void drawGui();

	void addInspector(ObjectType type, CustomInspector* inspector);

	void drawBasics();
	void drawTags();
	void drawTransform();
	void drawComponents();

	void drawLight(Light light);
	void drawCamera(Camera camera);
	void drawProjector(Projector projector);

	void drawMesh(Mesh mesh);
	void drawRenderer(Renderer renderer);

private:
	Entity target_;
	QGLWidget* view_;

	std::vector<std::pair<ObjectType, CustomInspector*>> inspectors_;
};
