#pragma once

#include <QDockWidget>

#include "entity.h"
#include "camera.h"
#include "../winbase.h"
#include "gizmospainter.h"
#include "frameeventlistener.h"
#include "entityloadedlistener.h"

class Canvas;
class StatsWidget;
class ImageEffect;
class CameraController;

class Game : public QDockWidget, public WinSingleton<Game>, public GizmosPainter, public EntityLoadedListener {
	Q_OBJECT

public:
	Game(QWidget* parent);
	~Game();

public:
	Canvas* canvas();

public:
	virtual void init(Ui::Suede* ui);
	virtual void awake();
	virtual void tick();

public:
	virtual void OnDrawGizmos();

public:
	virtual void OnEntityImported(Entity root, const std::string& path);

// public:
// 	virtual void OnFrameLeave();

private slots:
	void updateStat();
	void onToggleStat(int state);

	void onShadingModeChanged(const QString& str);

	void onFocusEntityBounds(Entity entity);
	void onSelectionChanged(const QList<Entity>& selected, const QList<Entity>& deselected);

protected:
	virtual void wheelEvent(QWheelEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void resizeEvent(QResizeEvent* event);
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void timerEvent(QTimerEvent *event);

protected:
	void updateSelection(QList<Entity>& container, const QList<Entity>& selected, const QList<Entity>& deselected);

private:
	void start();
	void createScene();
	float calculateCameraDistanceFitsBounds(Camera camera, Entity entity);

	void initializeStatWidget();

private:
	QList<Entity> selection_;
	/*RenderTexture targetTexture_;*/

	QTimer* timer_;

	Canvas* canvas_;
	StatsWidget* stat_;

	ImageEffect* grayscale_;
	ImageEffect* inversion_;
	ImageEffect* gaussianBlur_;

	CameraController* controller_;
};

