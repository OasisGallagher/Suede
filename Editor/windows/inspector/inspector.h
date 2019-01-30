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
	GameObject target_;
	QGLWidget* view_;

	uint blackTextureID_;
	std::map<ObjectType, std::shared_ptr<ComponentMetaObject>> suedeMetaObjects_;
};

template <class T>
void Inspector::drawBuiltinType(QObject* object, const char* name, bool(*draw)(const char*, T&, T, T)) {
	T value = object->property(name).value<T>();
	if (draw(name, value, std::numeric_limits<T>::lowest(), std::numeric_limits<T>::max())) {
		object->setProperty(name, value);
	}
}

template <class T>
inline void Inspector::drawUserVectorType(QObject* object, const char* name, bool(*draw)(const char*, T&)) {
	T value = object->property(name).value<T>();
	if (draw(name, value)) {
		object->setProperty(name, QVariant::fromValue(value));
	}
}

template <class T>
inline void Inspector::drawUserEnumType(QObject* object, const char* name) {
	int selected = -1;
	T value = object->property(name).value<T>();
	if (GUI::EnumPopup(name, +value, selected)) {
		object->setProperty(name, QVariant::fromValue(T::value(selected)));
	}
}

template <class T>
inline void Inspector::drawUserRangeType(QObject* object, const char* name, bool(*draw)(const char*, T&, T, T)) {
	range<T> r = object->property(name).value <range<T>>();
	T value = r.get_value();
	if (draw(name, value, r.min(), r.max())) {
		object->setProperty(name, QVariant::fromValue(r = value));
	}
}
