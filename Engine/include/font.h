#pragma once
#include "object.h"
#include "material.h"
#include "tools/event.h"

struct CharacterInfo {
	uint width;
	uint height;
	Vector4 texCoord;
};

class SUEDE_API Font : public Object {
	SUEDE_DEFINE_METATABLE_NAME(Font)
	SUEDE_DECLARE_IMPLEMENTATION(Font)

public:
	Font();

public:
	bool Load(const std::string& path, int size);
	bool Require(const std::wstring& str);

	uint GetFontSize() const;
	Texture2D* GetTexture() const;
	
	std::string GetFamilyName() const;
	std::string GetStyleName() const;

	Material* GetMaterial();
	bool GetCharacterInfo(wchar_t wch, CharacterInfo* info);

public:
	event<> materialRebuilt;
};
