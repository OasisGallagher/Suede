#pragma once
#include <QObject>
#include <QVector>

#include <algorithm>
#include <glm/glm.hpp>

#include "rect.h"
#include "color.h"
#include "ranged.h"
#include "component.h"

#include "texture.h"
#include "material.h"

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

Q_DECLARE_METATYPE(glm::vec2)
Q_DECLARE_METATYPE(glm::vec3)
Q_DECLARE_METATYPE(glm::vec4)

Q_DECLARE_METATYPE(iranged)
Q_DECLARE_METATYPE(franged)

Q_DECLARE_METATYPE(Rect)
Q_DECLARE_METATYPE(Color)

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)
Q_DECLARE_METATYPE(Material)
Q_DECLARE_METATYPE(RenderTexture)

#include "transform.h"

class TransformMetaObject : public ComponentMetaObjectT<Transform> {
	Q_OBJECT
	SUEDE_NAMED_PROPERTY(glm::vec3, Position, LocalPosition)
	SUEDE_NAMED_PROPERTY(glm::vec3, Rotation, LocalEulerAngles)
	SUEDE_NAMED_PROPERTY(glm::vec3, Scale, LocalScale)
};

#include "camera.h"

Q_DECLARE_METATYPE(ClearType)
Q_DECLARE_METATYPE(RenderPath)
Q_DECLARE_METATYPE(DepthTextureMode)

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

#include "rigidbody.h"

class RigidbodyMetaObject : public ComponentMetaObjectT<Rigidbody> {
	Q_OBJECT
	SUEDE_PROPERTY(float, Mass)
	SUEDE_PROPERTY(glm::vec3, Velocity)
};

#include "light.h"

Q_DECLARE_METATYPE(LightType)
Q_DECLARE_METATYPE(LightImportance)

class LightMetaObject : public ComponentMetaObjectT<Light> {
	Q_OBJECT

	SUEDE_PROPERTY(LightType, Type)
	SUEDE_PROPERTY(LightImportance, Importance)
	SUEDE_PROPERTY(Color, Color)
	SUEDE_PROPERTY(float, Intensity)
};

#include "renderer.h"

class RendererMetaObject : public ComponentMetaObjectT<Renderer> {
	Q_OBJECT
	Q_PROPERTY(QVector<Material> Materials READ GetMaterials)

public:
	QVector<Material> GetMaterials() {
		QVector<Material> answer;
		auto materials = target->GetMaterials();
		std::copy(materials.begin(), materials.end(), std::back_inserter(answer));
		return answer;
	}
};

class MeshRendererMetaObject : public RendererMetaObject {
	Q_OBJECT
};

class SkinnedMeshRendererMetaObject : public MeshRendererMetaObject {
	Q_OBJECT
};
