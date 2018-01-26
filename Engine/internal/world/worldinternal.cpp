#include "resources.h"
#include "tools/math2.h"
#include "worldinternal.h"
#include "internal/memory/factory.h"
#include "internal/file/assetimporter.h"
#include "internal/sprites/spriteinternal.h"
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

WorldInternal::WorldInternal()
	: ObjectInternal(ObjectTypeWorld)
	, environment_(MEMORY_CREATE(EnvironmentInternal))
	, root_(Factory::Create<SpriteInternal>()) {
}

Object WorldInternal::Create(ObjectType type) {
	Object object = Factory::Create(type);
	if (type >= ObjectTypeSprite) {
		Sprite sprite = dsp_cast<Sprite>(object);
		sprite->SetParent(GetRootSprite());
		sprites_.insert(std::make_pair(sprite->GetInstanceID(), sprite));

		SpriteCreatedEvent e;
		e.sprite = sprite;
		FireEvent(&e);
	}

	if (type >= ObjectTypeSpotLight && type <= ObjectTypeDirectionalLight) {
		lights_.insert(dsp_cast<Light>(object));
	}

	if (type == ObjectTypeCamera) {
		cameras_.insert(dsp_cast<Camera>(object));
	}

	return object;
}

Sprite WorldInternal::Import(const std::string& path) {
	AssetImporter importer;
	return importer.Import(path);
}

Sprite WorldInternal::GetSprite(uint id) {
	SpriteContainer::iterator ite = sprites_.find(id);
	if (ite == sprites_.end()) { return nullptr; }
	return ite->second;
}

bool WorldInternal::GetSprites(ObjectType type, std::vector<Sprite>& sprites) {
	if (type < ObjectTypeSprite) {
		Debug::LogError("invalid sprite type");
		return false;
	}

	if (type == ObjectTypeSprite) {
		for (SpriteContainer::iterator ite = sprites_.begin(); ite != sprites_.end(); ++ite) {
			sprites.push_back(ite->second);
		}
	}
	else if (type == ObjectTypeCamera) {
		sprites.assign(cameras_.begin(), cameras_.end());
	}
	else if (type == ObjectTypeLights) {
		sprites.assign(lights_.begin(), lights_.end());
	}
	else {
		for (SpriteContainer::iterator ite = sprites_.begin(); ite != sprites_.end(); ++ite) {
			if (ite->second->GetType() == type) {
				sprites.push_back(ite->second);
			}
		}
	}

	return !sprites.empty();
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

void WorldInternal::FireEvent(const WorldEventBase* e) {
	for (int i = 0; i < listeners_.size(); ++i) {
		listeners_[i]->OnWorldEvent(e);
	}
}

void WorldInternal::Update() {
	for (SpriteContainer::iterator ite = sprites_.begin(); ite != sprites_.end(); ++ite) {
		if (ite->second->GetActive()) {
			ite->second->Update();
		}
	}

	for (CameraContainer::iterator ite = cameras_.begin(); ite != cameras_.end(); ++ite) {
		if ((*ite)->GetActive()) {
			(*ite)->Render();
		}
	}
}
