#pragma once
#include <QWidget>

class GUI {
public:
	static void initialize(QWidget*widget);
	static void destroy();

	static void begin();
	static void end();

	static void text(const char* title);
	static bool input(const char* title, std::string& v);

	static void image(const char* title, uint texture);
	static bool imageButton(const char* title, uint texture);

	static void sameline();
	static void separator();

	static bool checkbox(const char* title, bool* v);

	template <class T>
	static bool combo(const char* title, int* selected, T first, T last);

	template <class T> static bool enums(const char* title, T value, int& selected);

	static bool slider(const char* title, float* v, float min, float max);

	static bool single(const char* title, float* v);
	static bool single2(const char* title, float* v);
	static bool single3(const char* title, float* v);
	static bool single4(const char* title, float* v);

	static bool color3(const char* title, float* v);
	static bool color4(const char* title, float* v);

	static bool collapsingHeader(const char* title);
};

#include "gui.inl"
