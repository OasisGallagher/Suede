#include "resources.h"
#include "tools/math2.h"
#include "worldinternal.h"
#include "internal/file/assetimporter.h"
#include "internal/base/transforminternal.h"
#include "internal/entities/entityinternal.h"
#include "internal/world/environmentinternal.h"

World& WorldInstance() {
	static World instance = Factory::Create<WorldInternal>();
	return instance;
}

bool WorldInternal::LightComparer::operator()(const Light & lhs, const Light & rhs) const {
	// Directional light > Importance > Luminance.
	ObjectType lt = lhs->GetType(), rt = rhs->GetType();
	if (lt != rt && (lt == ObjectTypeDirectionalLight || rt == ObjectTypeDirectionalLight)) {
		return lt == ObjectTypeDirectionalLight;
	}

	LightImportance lli = lhs->GetImportance(), rli = rhs->GetImportance();
	if (lli != rli) {
		return lli > rli;
	}

	return Math::Luminance(lhs->GetColor()) > Math::Luminance(rhs->GetColor());
}

bool WorldInternal::CameraComparer::operator() (const Camera& lhs, const Camera& rhs) const {
	return lhs->GetDepth() < rhs->GetDepth();
}

#include "internal/geometry/geometryutility.h"

WorldInternal::WorldInternal()
	: ObjectInternal(ObjectTypeWorld)
	, environment_(MEMORY_CREATE(EnvironmentInternal))
	, root_(Factory::Create<EntityInternal>()) {
	Transform transform = Factory::Create<TransformInternal>();
	root_->SetTransform(transform);
}

Object WorldInternal::Create(ObjectType type) {
	Object object = Factory::Create(type);
	if (type >= ObjectTypeEntity) {
		Entity entity = dsp_cast<Entity>(object);
		Transform transform = Factory::Create<TransformInternal>();
		entity->SetTransform(transform);

		EntityCreatedEventPointer e = NewWorldEvent<EntityCreatedEventPointer>();
		e->entity = entity;
		FireEvent(e);

		transform->SetParent(root_->GetTransform());
		entities_.insert(std::make_pair(entity->GetInstanceID(), entity));
	}

	if (type >= ObjectTypeSpotLight && type <= ObjectTypeDirectionalLight) {
		lights_.insert(dsp_cast<Light>(object));
	}

	if (type == ObjectTypeCamera) {
		cameras_.insert(dsp_cast<Camera>(object));
	}

	return object;
}

Entity WorldInternal::Import(const std::string& path) {
	AssetImporter importer;
	return importer.Import(path);
}

Entity WorldInternal::GetEntity(uint id) {
	EntityContainer::iterator ite = entities_.find(id);
	if (ite == entities_.end()) { return nullptr; }
	return ite->second;
}

bool WorldInternal::GetEntities(ObjectType type, std::vector<Entity>& entities) {
	if (type < ObjectTypeEntity) {
		Debug::LogError("invalid entity type");
		return false;
	}

	if (type == ObjectTypeEntity) {
		for (EntityContainer::iterator ite = entities_.begin(); ite != entities_.end(); ++ite) {
			entities.push_back(ite->second);
		}
	}
	else if (type == ObjectTypeCamera) {
		entities.assign(cameras_.begin(), cameras_.end());
	}
	else if (type == ObjectTypeLights) {
		entities.assign(lights_.begin(), lights_.end());
	}
	else {
		for (EntityContainer::iterator ite = entities_.begin(); ite != entities_.end(); ++ite) {
			if (ite->second->GetType() == type) {
				entities.push_back(ite->second);
			}
		}
	}

	return !entities.empty();
}

void WorldInternal::AddEventListener(WorldEventListener* listener) {
	if (listener == nullptr) {
		Debug::LogError("invalid world event listener.");
		return;
	}

	if (std::find(listeners_.begin(), listeners_.end(), listener) == listeners_.end()) {
		listeners_.push_back(listener);
	}
}

void WorldInternal::RemoveEventListener(WorldEventListener* listener) {
	EventListenerContainer::iterator pos = std::find(listeners_.begin(), listeners_.end(), listener);
	if (pos != listeners_.end()) {
		listeners_.erase(pos);
	}
}

bool WorldInternal::FireEvent(WorldEventBasePointer e) {
	for (WorldEventContainer::const_iterator ite = events_.begin(); ite != events_.end(); ++ite) {
		if ((*ite)->Equals(e)) {
			return false;
		}
	}

	events_.push_back(e);
	return true;
}

void WorldInternal::FireEventImmediate(WorldEventBasePointer e) {
	for (int i = 0; i < listeners_.size(); ++i) {
		listeners_[i]->OnWorldEvent(e);
	}
}

#include "internal/geometry/plane.h"
#include "internal/geometry/geometryutility.h"

void WorldInternal::Update() {
	for (WorldEventContainer::const_iterator ite = events_.begin(); ite != events_.end(); ++ite) {
		FireEventImmediate(*ite);
	}

	events_.clear();

	Entity room;
	for (EntityContainer::iterator ite = entities_.begin(); ite != entities_.end(); ++ite) {
		if (ite->second->GetName() == "room") {
			room = ite->second;
		}

		if (ite->second->GetActive()) {
			ite->second->Update();
		}
	}

	for (CameraContainer::iterator ite = cameras_.begin(); ite != cameras_.end(); ++ite) {
		if ((*ite)->GetActive()) {
			(*ite)->Render();
		}
	}

	if (!room || cameras_.empty()) {
		return;
	}

	Mesh mesh = room->GetMesh();
	if (!mesh) {
		return;
	}

	const std::vector<uint>& indexes = mesh->GetIndexes();
	const std::vector<glm::vec3>& vertices = mesh->GetVertices();
	Plane planes[6];
	Camera camera = *cameras_.begin();
	GeometryUtility::CalculateFrustumPlanes(planes, camera->GetProjectionMatrix() * camera->GetTransform()->GetWorldToLocalMatrix());
	std::vector<glm::vec3> polygon;

	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		SubMesh subMesh = mesh->GetSubMesh(i);
		uint indexCount, baseVertex, baseIndex;
		subMesh->GetTriangles(indexCount, baseVertex, baseIndex);
		// TODO: triangle strip.
		for (int j = 0; j < indexCount; j += 3) {
			uint index0 = indexes[baseIndex] + baseVertex;
			uint index1 = indexes[baseIndex + 1] + baseVertex;
			uint index2 = indexes[baseIndex + 2] + baseVertex;

			glm::vec3 vs[] = {
				vertices[index0],
				vertices[index1],
				vertices[index2]
			};

			GeometryUtility::ClampTriangle(polygon, vs, planes, CountOf(planes));
		}
	}
}
