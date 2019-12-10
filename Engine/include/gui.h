#pragma once
#include "rect.h"
#include "color.h"

#include <string>

/**
 * @brief immediate mode GUI.
 */
class SUEDE_API GUI {
public:
	static void LoadFont(const char* file);

	static void Begin(uint w, uint h, const Color& foregroundColor, const Color& backgroundColor);
	static void End();

	static void LabelField(const char* text);
	static void LabelField(const char* title, const char* text);

	static bool TextField(const char* title, std::string& value);

	static bool Button(const char* title);

	static void Image(const char* title, uint texture);
	static bool ImageButton(const char* title, uint texture);

	static void Sameline();
	static void Separator();

	static void Indent(float value = 4);
	static void Unindent(float value = 4);

	static bool Toggle(const char* title, bool& value);

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

	static bool Slider(const char* title, float& value, float min, float max);
	static bool IntSlider(const char* title, int& value, int min, int max);

	static bool IntField(const char* title, int& value, int min = std::numeric_limits<int>::lowest(), int max = std::numeric_limits<int>::max());
	static bool UIntField(const char* title, uint& value, uint min = std::numeric_limits<uint>::lowest(), uint max = std::numeric_limits<uint>::max());

	static bool FloatField(const char* title, float& value, float min = std::numeric_limits<float>::lowest(), float max = std::numeric_limits<float>::max());
	static bool Float2Field(const char* title, Vector2& value);
	static bool Float3Field(const char* title, Vector3& value);
	static bool Float4Field(const char* title, Vector4& value);

	static bool ColorField(const char* title, Color& value);

	static bool RectField(const char* title, Rect& value);
	static bool NormalizedRectField(const char* title, Rect& value);

	static bool BeginMenu(const char* title);
	static bool MenuItem(const char* title, bool selected);
	static void EndMenu();

	static bool CollapsingHeader(const char* title);

	static void BeginScope(int id);
	static void BeginScope(const char* id);

	static void EndScope();
};

#include "gui.inl"
