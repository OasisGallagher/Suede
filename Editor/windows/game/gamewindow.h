#pragma once

#include <QDockWidget>

#include "camera.h"

#include "gameobject.h"
#include "gizmospainter.h"
#include "qtinputdelegate.h"
#include "main/childwindow.h"

class Editor;

class Canvas;
class StatsWidget;
class SelectionGizmos;
class CameraController;

class GameWindow : public ChildWindow {
	Q_OBJECT

public:
	enum {
		WindowType = ChildWindowType::Game,
	};

public:
	GameWindow(QWidget* parent);

public:
	Canvas* canvas() { return canvas_; }

public:
	virtual void initUI();
	virtual void awake();
	virtual void tick();

private slots:
	void onToggleStat(int state);

	void onShadingModeChanged(const QString& str);

	void onFocusGameObjectBounds(GameObject* go);
	void onSelectionChanged(const QList<GameObject*>& selected, const QList<GameObject*>& deselected);

protected:
	virtual void resizeEvent(QResizeEvent* event);
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void timerEvent(QTimerEvent *event);

private:
	void createScene();
	float calculateCameraDistanceFitsBounds(Camera* camera, const Bounds& bounds);

	void updateStatPosition();
	void onGameObjectImported(GameObject* root, const std::string& path);

private:
	QtInputDelegate* input_;
	Canvas* canvas_;
	StatsWidget* stat_;

	SelectionGizmos* gizmos_;
	CameraController* controller_;
};
