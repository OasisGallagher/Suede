#include "QtImGui.h"
#include "ImGuiRenderer.h"
#include <QtOpenGL/QGLWidget>

namespace QtImGui {

namespace {

class QWidgetWindowWrapper : public WindowWrapper {
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

}

static std::map<QGLWidget*, ImGuiRenderer*> renderers_;

void create(QGLWidget *widget) {
	if (renderers_.find(widget) == renderers_.end()) {
		ImGuiRenderer* renderer = renderers_[widget] = new ImGuiRenderer;
		renderer->initialize(new QWidgetWindowWrapper(widget));
	}
}

void newFrame(QGLWidget* widget) {
	IM_ASSERT(renderers_.find(widget) != renderers_.end());
    renderers_[widget]->newFrame();
}

void destroy(QGLWidget* widget) {
	renderers_[widget]->destroy();

	delete renderers_[widget];
	renderers_.erase(widget);
}

void destroyAll() {
	for (auto p : renderers_) {
		p.second->destroy();
		delete p.second;
	}

	renderers_.clear();
}

} // namespace QtImGui
