#pragma once
#include <vector>
#include "sprite.h"
#include "object.h"
#include "environment.h"

class ENGINE_EXPORT IWorld : virtual public IObject {
public:
	virtual void Initialize() = 0;

	virtual void Update() = 0;
	virtual Object Create(ObjectType type) = 0;

	virtual Sprite Import(const std::string& path) = 0;

	virtual Sprite GetRootSprite() = 0;
	virtual Sprite GetSprite(unsigned id) = 0;
	virtual bool GetSprites(ObjectType type, std::vector<Sprite>& sprites) = 0;

	virtual Environment GetEnvironment() = 0;
};

DEFINE_OBJECT_PTR(World);
