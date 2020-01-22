#pragma once

#include "imgui.h"

#include <QObject>
#include <QPoint>
#include <QTimer>
#include <memory>
#include <GL/glew.h>

class QMouseEvent;
class QWheelEvent;
class QKeyEvent;

class ImGuiWindowWrapper {
public:
	virtual ~ImGuiWindowWrapper() {}
	virtual void installEventFilter(QObject *object) = 0;
	virtual QSize size() const = 0;
	virtual qreal devicePixelRatio() const = 0;
	virtual bool isActive() const = 0;
	virtual QWidget* widget() = 0;
	virtual QPoint mapFromGlobal(const QPoint &p) const = 0;
};

class ImGuiRenderer : public QObject {
    Q_OBJECT

public:
	ImGuiRenderer() {}

public:
    void initialize(ImGuiWindowWrapper *window, ImFontAtlas* fontAtlas);
	void destroy();

    void newFrame();
	ImGuiContext* context() { return m_context; }

	bool eventFilter(QObject *watched, QEvent *event);


private:
    void onKeyEvent(QKeyEvent *event);
	void onMouseEvent(QMouseEvent *event);
	void onWheelEvent(QWheelEvent *event);

    void renderDrawList(ImDrawData *draw_data);
    bool createFontsTexture();
    bool createDeviceObjects();

	ImGuiContext* m_context;
	std::unique_ptr<ImGuiWindowWrapper> m_window;

    double       m_LastTime = 0.0f;
    bool         m_MousePressed[3] = { false, false, false };
    float        m_MouseWheel;
    float        m_MouseWheelH;
    GLuint       m_FontTexture = 0;
    int          m_ShaderHandle = 0, m_VertHandle = 0, m_FragHandle = 0;
    int          m_AttribLocationTex = 0, m_AttribLocationProjMtx = 0;
    int          m_AttribLocationPosition = 0, m_AttribLocationUV = 0, m_AttribLocationColor = 0;
    unsigned int m_VboHandle = 0, m_VaoHandle = 0, m_ElementsHandle = 0;
};
