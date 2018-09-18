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

#define SUEDE_NAMED_PROPERTY(type, name, property)	Q_PROPERTY(type name READ Get ## property WRITE Set ## property) \
	type Get ## property() { return target->Get ## property(); } \
	void Set ## property(type value) { target->Set ## property(value); }

#define SUEDE_PROPERTY(type, name)	SUEDE_NAMED_PROPERTY(type, name, name)

Q_DECLARE_METATYPE(Color)
Q_DECLARE_METATYPE(glm::vec2)
Q_DECLARE_METATYPE(glm::vec3)
Q_DECLARE_METATYPE(glm::vec4)

#include "transform.h"

class TransformMetaObject : public ComponentMetaObjectT<Transform> {
	Q_OBJECT
	SUEDE_NAMED_PROPERTY(glm::vec3, Position, LocalPosition)
	SUEDE_NAMED_PROPERTY(glm::vec3, Rotation, LocalEulerAngles)
	SUEDE_NAMED_PROPERTY(glm::vec3, Scale, LocalScale)
};

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

#include "renderer.h"

class MeshRendererMetaObject : public ComponentMetaObjectT<MeshRenderer> {

};
