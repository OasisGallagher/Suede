#include "QtImGui.h"
#include "ImGuiRenderer.h"
#include <QtOpenGL/QGLWidget>

class QWidgetWindowWrapper : public ImGuiWindowWrapper {
public:
    QWidgetWindowWrapper(QGLWidget *w) : w(w) {}

    void installEventFilter(QObject *object) override {
        return w->installEventFilter(object);
    }

    QSize size() const override {
        return w->size();
    }

    qreal devicePixelRatio() const override {
        return w->devicePixelRatioF();
    }

    bool isActive() const override {
        return w->isActiveWindow();
    }

    QPoint mapFromGlobal(const QPoint &p) const override {
        return w->mapFromGlobal(p);
    }

private:
	QGLWidget *w;
};

QtImGui::QtImGui(const char* fontFile, int fontSize) {
	fontAtlas_ = new ImFontAtlas();
	fontAtlas_->AddFontFromFileTTF(fontFile, fontSize, nullptr, fontAtlas_->GetGlyphRangesChineseFull());
}

QtImGui::~QtImGui() {
	delete fontAtlas_;
}

void QtImGui::registe(QGLWidget *widget) {
	if (renderers_.find(widget) == renderers_.end()) {
		widget->setFocusPolicy(Qt::StrongFocus);
		ImGuiRenderer* renderer = renderers_[widget] = new ImGuiRenderer;
		renderer->initialize(new QWidgetWindowWrapper(widget), fontAtlas_);
	}
}

void QtImGui::newFrame(QGLWidget* widget) {
	IM_ASSERT(renderers_.find(widget) != renderers_.end());
    renderers_[widget]->newFrame();
}

void QtImGui::unregister(QGLWidget* widget) {
	renderers_[widget]->destroy();

	delete renderers_[widget];
	renderers_.erase(widget);
}

void QtImGui::unregisterAll() {
	for (auto p : renderers_) {
		p.second->destroy();
		delete p.second;
	}

	renderers_.clear();
}
