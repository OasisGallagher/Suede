#pragma once
#include "object.h"
#include "material.h"
#include "tools/event.h"

struct CharacterInfo {
	float minX = 0;
	float minY = 0;
	float maxX = 0;
	float maxY = 0;

	float advance = 0;

	Vector2 uvLeftTop;
	Vector2 uvLeftBottom;
	Vector2 uvRightTop;
	Vector2 uvRightBottom;
};

class SUEDE_API Font : public Object {
	SUEDE_DEFINE_METATABLE_NAME(Font)
	SUEDE_DECLARE_IMPLEMENTATION(Font)

public:
	Font();

public:
	bool Load(const std::string& path);

	void SetPixelHeight(float value);
	float GetPixelHeight() const;

	Material* GetMaterial();
	Texture2D* GetAtlasTexture();

	bool GetCharacterInfo(int codepoint, CharacterInfo* info);
	bool RequestCharactersInTexture(const std::vector<int>& codepoints);

	int GetAscent() const;
	int GetDecent() const;
	int GetLineGap() const;
	float GetKerning(int codepoint0, int codepoint1) const;

public:
	event<> materialRebuilt;
};
