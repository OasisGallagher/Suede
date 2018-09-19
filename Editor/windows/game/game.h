#pragma once

#include <QDockWidget>

#include "camera.h"
#include "../winbase.h"
#include "gameobject.h"
#include "gizmospainter.h"
#include "frameeventlistener.h"
#include "gameobjectloadedlistener.h"

class Canvas;
class StatsWidget;
class CameraController;

class Game : public QDockWidget, public WinSingleton<Game>, public GizmosPainter, public GameObjectLoadedListener {
	Q_OBJECT

public:
	Game(QWidget* parent);
	~Game();

public:
	Canvas* canvas();

public:
	virtual void init(Ui::Editor* ui);
	virtual void awake();
	virtual void tick();

public:
	virtual void OnDrawGizmos();

public:
	virtual void OnGameObjectImported(GameObject root, const std::string& path);

private slots:
	void updateStatContent();
	void onToggleStat(int state);

	void onShadingModeChanged(const QString& str);

	void onFocusGameObjectBounds(GameObject go);
	void onSelectionChanged(const QList<GameObject>& selected, const QList<GameObject>& deselected);

protected:
	virtual void wheelEvent(QWheelEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void resizeEvent(QResizeEvent* event);
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void timerEvent(QTimerEvent *event);

protected:
	void updateSelection(QList<GameObject>& container, const QList<GameObject>& selected, const QList<GameObject>& deselected);

private:
	void start();
	void createScene();
	float calculateCameraDistanceFitsBounds(Camera camera, GameObject go);

	void updateStatPosition();

private:
	QList<GameObject> selection_;
	/*RenderTexture targetTexture_;*/

	QTimer* timer_;

	Canvas* canvas_;
	StatsWidget* stat_;

	CameraController* controller_;
};
