#pragma once
#include <QDockWidget>

#include "light.h"
#include "camera.h"
#include "texture.h"
#include "renderer.h"
#include "projector.h"
#include "main/childwindow.h"

#include "custom/componentmetaobject.h"

class IMGUIWidget;
class InspectorWindow : public ChildWindow {
	Q_OBJECT

public:
	enum {
		WindowType = ChildWindowType::Inspector,
	};

public:
	InspectorWindow(QWidget* parent);
	~InspectorWindow();

public:
	virtual void initUI();
	virtual void awake();
	virtual void tick();

private:
	void onGui();
	void drawGui(GameObject* go);

	QObject* componentMetaObject(Component* component, std::string& typeName);
	void addSuedeMetaObject(ObjectType type, std::shared_ptr<ComponentMetaObject> mo);

	void drawBasics(GameObject* go);
	void drawTags(GameObject* go);
	void drawComponents(GameObject* go);

	void drawMetaObject(QObject* object);

	void drawUserType(const QMetaProperty& p, QObject* object, const char* name);

	void drawMaterialVector(QObject* object, const char* name);

	void drawBuiltinType(const QMetaProperty& p, QObject* object, const char* name);

	template <class T>
	void drawBuiltinType(QObject* object, const char* name, bool(*draw)(const char*, T&, T, T));

	template <class T>
	void drawUserEnumType(QObject* object, const char* name);

	template <class T>
	void drawUserVectorType(QObject* object, const char* name, bool(*draw)(const char*, T&));

	template <class T>
	void drawUserRangeType(QObject* object, const char* name, bool(*draw)(const char*, T&, T, T));

private:
	IMGUIWidget* view_;

	uint blackTextureID_;
	std::map<ObjectType, std::shared_ptr<ComponentMetaObject>> suedeMetaObjects_;
};

template <class T>
void InspectorWindow::drawBuiltinType(QObject* object, const char* name, bool(*draw)(const char*, T&, T, T)) {
	T value = object->property(name).value<T>();
	if (draw(name, value, std::numeric_limits<T>::lowest(), std::numeric_limits<T>::max())) {
		object->setProperty(name, value);
	}
}

template <class T>
inline void InspectorWindow::drawUserVectorType(QObject* object, const char* name, bool(*draw)(const char*, T&)) {
	T value = object->property(name).value<T>();
	if (draw(name, value)) {
		object->setProperty(name, QVariant::fromValue(value));
	}
}

template <class T>
inline void InspectorWindow::drawUserEnumType(QObject* object, const char* name) {
	int selected = -1;
	T value = object->property(name).value<T>();
	if (GUI::EnumPopup(name, +value, selected)) {
		object->setProperty(name, QVariant::fromValue(T::value(selected)));
	}
}

template <class T>
inline void InspectorWindow::drawUserRangeType(QObject* object, const char* name, bool(*draw)(const char*, T&, T, T)) {
	ranged<T> r = object->property(name).value <ranged<T>>();
	T value = r.get_value();
	if (draw(name, value, r.min(), r.max())) {
		object->setProperty(name, QVariant::fromValue(r = value));
	}
}
