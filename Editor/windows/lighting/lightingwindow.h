#pragma once

#include <QDockWidget>

#include "main/childwindow.h"

class LightingWindow : public ChildWindow {
	Q_OBJECT

public:
	enum {
		WindowType = ChildWindowType::Lighting,
	};

public:
	LightingWindow(QWidget* parent);

public:
	virtual void awake();

protected:
	virtual void showEvent(QShowEvent* event);

private slots:
	void onAmbientChanged(const QColor& color);
	void onOcclusionChanged(int state);

	void onFogColorChanged(const QColor& color);
	void onFogDensityChanged(float density);
};
