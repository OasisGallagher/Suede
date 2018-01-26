#pragma once
#include <glm/glm.hpp>

#include "object.h"
#include "material.h"

SUEDE_DEFINE_OBJECT_POINTER(Font);
SUEDE_DECLARE_OBJECT_CREATER(Font);

struct CharacterInfo {
	uint width;
	uint height;
	glm::vec4 texCoord;
};

class SUEDE_API IFont : virtual public IObject {
public:
	virtual bool Load(const std::string& path, int size) = 0;
	virtual bool Require(const std::wstring& str) = 0;

	virtual uint GetFontSize() const = 0;
	virtual Texture2D GetTexture() const = 0;
	
	virtual std::string GetFamilyName() const = 0;
	virtual std::string GetStyleName() const = 0;

	virtual Material GetMaterial() = 0;
	virtual bool GetCharacterInfo(wchar_t wch, CharacterInfo* info) = 0;
};
