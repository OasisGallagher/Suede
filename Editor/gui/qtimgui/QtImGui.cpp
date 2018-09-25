#include "QtImGui.h"
#include "ImGuiRenderer.h"
#include <QWindow>
#ifdef QT_WIDGETS_LIB
#include <QWidget>
#endif

namespace QtImGui {

#ifdef QT_WIDGETS_LIB

namespace {

class QWidgetWindowWrapper : public WindowWrapper {
public:
    QWidgetWindowWrapper(QWidget *w) : w(w) {}
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
    QWidget *w;
};

}

static std::map<QWidget*, ImGuiRenderer*> renderers_;

void initialize(QWidget *widget) {
	if (renderers_.find(widget) == renderers_.end()) {
		ImGuiRenderer* renderer = renderers_[widget] = new ImGuiRenderer;
		renderer->initialize(new QWidgetWindowWrapper(widget));
	}
}

#endif

namespace {

class QWindowWindowWrapper : public WindowWrapper {
public:
    QWindowWindowWrapper(QWindow *w) : w(w) {}
    void installEventFilter(QObject *object) override {
        return w->installEventFilter(object);
    }
    QSize size() const override {
        return w->size();
    }
    qreal devicePixelRatio() const override {
        return w->devicePixelRatio();
    }
    bool isActive() const override {
        return w->isActive();
    }
    QPoint mapFromGlobal(const QPoint &p) const override {
        return w->mapFromGlobal(p);
    }
private:
    QWindow *w;
};

}

void newFrame(QWidget* widget) {
	IM_ASSERT(renderers_.find(widget) != renderers_.end());
    renderers_[widget]->newFrame();
}

void destroy(QWidget* widget) {
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

}
