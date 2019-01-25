#pragma once

#include <QDockWidget>

#include "camera.h"
#include "../winbase.h"
#include "gameobject.h"
#include "gizmospainter.h"
#include "qtinputdelegate.h"
#include "frameeventlistener.h"
#include "gameobjectimportedlistener.h"

class Canvas;
class StatsWidget;
class SelectionGizmos;
class CameraController;

class Game : public QDockWidget, public WinSingleton<Game>, public GameObjectImportedListener {
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
	virtual void OnGameObjectImported(GameObject root, const std::string& path);

private slots:
	void updateStatContent();
	void onToggleStat(int state);

	void onShadingModeChanged(const QString& str);

	void onFocusGameObjectBounds(GameObject go);
	void onSelectionChanged(const QList<GameObject>& selected, const QList<GameObject>& deselected);

protected:
	virtual void resizeEvent(QResizeEvent* event);
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void timerEvent(QTimerEvent *event);

private:
	void createScene();
	float calculateCameraDistanceFitsBounds(Camera camera, const Bounds& bounds);

	void updateStatPosition();

private:
	/*RenderTexture targetTexture_;*/
	QTimer* timer_;

	QtInputDelegate* input_;
	Canvas* canvas_;
	StatsWidget* stat_;

	SelectionGizmos* gizmos_;
	CameraController* controller_;
};
