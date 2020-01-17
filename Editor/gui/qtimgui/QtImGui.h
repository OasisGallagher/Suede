#pragma once
#include <map>

class QGLWidget;
class QtImGui {
public:
	QtImGui(const char* fontFile, int fontSize);
	~QtImGui();

public:
	void registe(QGLWidget *window);
	void newFrame(QGLWidget* widget);
	void unregister(QGLWidget* widget);
	void unregisterAll();

private:
	class ImFontAtlas* fontAtlas_;
	std::map<QGLWidget*, class ImGuiRenderer*> renderers_;
};
