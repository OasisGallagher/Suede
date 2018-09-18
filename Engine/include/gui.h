#pragma once
#include "types.h"
#include "color.h"

#include <string>
#include <glm/glm.hpp>

/**
 * @brief immediate mode GUI.
 */
class SUEDE_API GUI {
public:
	static void LoadFont(const char* file);

	static void Begin(uint w, uint h, glm::vec3 backgroundColor);
	static void End();

	static void LabelField(const char* text);
	static void LabelField(const char* title, const char* text);

	static bool TextField(const char* title, std::string& v);

	static void Image(const char* title, uint texture);
	static bool ImageButton(const char* title, uint texture);

	static void Sameline();
	static void Separator();

	static void Indent(float v = 4);
	static void Unindent(float v = 4);

	static bool Toggle(const char* title, bool& v);

	template <class T>
	static bool EnumPopup(const char* title, T value, int& selected);

	template <class T>
	static bool Popup(const char* title, int* selected, T first, T last);

	/** @param items '\0' separated string, ends with "\0\0". */
	static bool Popup(const char* title, int* selected, const char* items);

	/** @param items '\0' separated string, ends with "\0\0". */
	static bool MaskPopup(const char* title, int mask, const char* items);

	template <class T>
	static bool EnumMaskPopup(const char* title, T value);

	static bool Slider(const char* title, float* v, float min, float max);

	static bool IntField(const char* title, int& v);

	static bool FloatField(const char* title, float& v);
	static bool Float2Field(const char* title, glm::vec2& v);
	static bool Float3Field(const char* title, glm::vec3& v);
	static bool Float4Field(const char* title, glm::vec4& v);

	static bool ColorField(const char* title, Color& v);

	static bool BeginMenu(const char* title);
	static bool MenuItem(const char* title, bool selected);
	static void EndMenu();

	static bool CollapsingHeader(const char* title);
};

#include "gui.inl"
