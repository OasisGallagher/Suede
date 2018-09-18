#pragma once
#include <QObject>
#include <glm/glm.hpp>

#include "color.h"
#include "component.h"

class ComponentMetaObject : public QObject {
	Q_OBJECT
public:
	virtual ~ComponentMetaObject() {}

public:
	virtual void setComponent(Component component) = 0;
};

template <class T>
class ComponentMetaObjectT : public ComponentMetaObject {
public:
	virtual void setComponent(Component component) {
		target = suede_dynamic_cast<T>(component);
	}

protected:
	T target;
};

#define SUEDE_PROPERTY(type, name)	Q_PROPERTY(type name READ Get ## name WRITE Set ## name) \
	type Get ## name() { return target->Get ## name(); } \
	void Set ## name(type value) { target->Set ## name(value); }

Q_DECLARE_METATYPE(Color)
Q_DECLARE_METATYPE(glm::vec2)
Q_DECLARE_METATYPE(glm::vec3)
Q_DECLARE_METATYPE(glm::vec4)

#include "camera.h"

class CameraMetaObject : public ComponentMetaObjectT<Camera> {
	Q_OBJECT

	SUEDE_PROPERTY(int, Depth)
	SUEDE_PROPERTY(bool, Perspective)
	SUEDE_PROPERTY(float, OrthographicSize)
	SUEDE_PROPERTY(ClearType, ClearType)
	SUEDE_PROPERTY(RenderPath, RenderPath)
	SUEDE_PROPERTY(DepthTextureMode, DepthTextureMode)
	SUEDE_PROPERTY(Color, ClearColor)
	SUEDE_PROPERTY(RenderTexture, TargetTexture)
	SUEDE_PROPERTY(float, Aspect)
	SUEDE_PROPERTY(float, NearClipPlane)
	SUEDE_PROPERTY(float, FarClipPlane)
	SUEDE_PROPERTY(float, FieldOfView)
	SUEDE_PROPERTY(Rect, Rect)
};

Q_DECLARE_METATYPE(ClearType)
Q_DECLARE_METATYPE(RenderPath)
Q_DECLARE_METATYPE(DepthTextureMode)

#include "light.h"

class LightMetaObject : public ComponentMetaObjectT<Light> {
	Q_OBJECT

	SUEDE_PROPERTY(LightImportance, Importance)
	SUEDE_PROPERTY(Color, Color)
	SUEDE_PROPERTY(float, Intensity)
};

Q_DECLARE_METATYPE(LightImportance)
