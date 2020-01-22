#pragma once

#include "bounds.h"
#include "camera.h"
#include "material.h"
#include "gameobject.h"
#include "gizmospainter.h"
#include "qtinputdelegate.h"
#include "main/childwindow.h"

class Input;
class Physics;
class Graphics;

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
	~GameWindow();

public:
	Canvas* canvas() { return canvas_; }

public:
	virtual void initUI();
	virtual void awake();
	virtual void tick();

private:
	void onToggleGizmos(bool checked);
	void onToggleStatistics(bool checked);
	void onToggleDrawPhysics(bool checked);

	void onShadingModeChanged(const QString& str);
	void onFocusGameObjectBounds(GameObject* go);

protected:
	virtual void resizeEvent(QResizeEvent* event);
	virtual void keyPressEvent(QKeyEvent* event);

private:
	void setupScene();
	float calculateCameraDistanceFitsBounds(Camera* camera, const Bounds& bounds);

	void updateStatPosition();

	void onFrameLeave();
	void onEngineScreenSizeChanged(uint w, uint h);

private:
	Input* input_;
	QtInputDelegate* inputDelegate_;

	Physics* physics_;
	Graphics* graphics_;

	Canvas* canvas_;
	StatsWidget* stat_;

	ref_ptr<Material> outlineMaterial_;
	ref_ptr<RenderTexture> selectCameraTargetTexture_;

	SelectionGizmos* selectionGizmos_;
	CameraController* controller_;
};
