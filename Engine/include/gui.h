#pragma once
#include <string>
#include "types.h"

class SUEDE_API GUI {
public:
	static void LoadFont(const char* file);

	static void Begin(uint w, uint h);
	static void End();

	static void Label(const char* title);
	static bool Text(const char* title, std::string& v);

	static void Image(const char* title, uint texture);
	static bool ImageButton(const char* title, uint texture);

	static void Sameline();
	static void Separator();

	static bool Toggle(const char* title, bool* v);

	template <class T>
	static bool Popup(const char* title, int* selected, T first, T last);

	/** @param items '\0' separated string, ends with "\0\0". */
	static bool Popup(const char* title, int* selected, const char* items);

	template <class T> static bool EnumPopup(const char* title, T value, int& selected);

	static bool Slider(const char* title, float* v, float min, float max);

	static bool Float(const char* title, float* v);
	static bool Float2(const char* title, float* v);
	static bool Float3(const char* title, float* v);
	static bool Float4(const char* title, float* v);

	static bool Color3(const char* title, float* v);
	static bool Color4(const char* title, float* v);

	static bool CollapsingHeader(const char* title);
};

#include "gui.inl"
