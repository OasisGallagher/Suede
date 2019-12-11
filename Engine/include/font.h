#pragma once
#include "object.h"
#include "material.h"

struct CharacterInfo {
	uint width;
	uint height;
	Vector4 texCoord;
};

class FontMaterialRebuiltListener {
public:
	virtual void OnMaterialRebuilt() = 0;
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

	void AddMaterialRebuiltListener(FontMaterialRebuiltListener* listener);
	void RemoveMaterialRebuiltListener(FontMaterialRebuiltListener* listener);
};
