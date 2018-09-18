#pragma once
#include <QDockWidget>

#include <glm/glm.hpp>

#include "light.h"
#include "camera.h"
#include "texture.h"
#include "renderer.h"
#include "projector.h"
#include "../winbase.h"

#include "custom/componentmetaobject.h"

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
	void onSelectionChanged(const QList<GameObject>& selected, const QList<GameObject>& deselected);

private:
	void onGui();
	void drawGui();

	QObject* componentMetaObject(Component component, std::string& typeName);
	void addSuedeMetaObject(ObjectType type, std::shared_ptr<ComponentMetaObject> mo);

	void drawBasics();
	void drawTags();
	void drawComponents();

	void drawMetaObject(QObject* object);

	void drawUserType(QMetaProperty &p, QObject* object, const char* name);
	void drawBuiltinType(QMetaProperty &p, QObject* object, const char* name);

private:
	GameObject target_;
	QGLWidget* view_;

	std::map<ObjectType, std::shared_ptr<ComponentMetaObject>> suedeMetaObjects_;
};
