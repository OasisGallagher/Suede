#pragma once
#include <map>

class QGLWidget;
class QtImGui {
public:
	QtImGui(const char* fontFile, int fontSize);
	~QtImGui();

public:
	void attach(QGLWidget *widget);
	void detach(QGLWidget* widget);
	void detachAll();

	void newFrame(QGLWidget* widget);

private:
	class ImFontAtlas* fontAtlas_;
	std::map<QGLWidget*, class ImGuiRenderer*> renderers_;
};
