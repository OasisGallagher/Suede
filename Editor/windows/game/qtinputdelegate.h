#pragma once
#include "input.h"
#include <QWidget>
#include <QMouseEvent>

class QtInputDelegate : public QObject, public InputInternal {
	Q_OBJECT

public:
	QtInputDelegate(QWidget* view);

public:
	virtual void OnFrameLeave();

	virtual bool GetKey(KeyCode key);
	virtual bool GetKeyUp(KeyCode key);
	virtual bool GetKeyDown(KeyCode key);

	virtual bool GetMouseButton(int button);
	virtual bool GetMouseButtonUp(int button);
	virtual bool GetMouseButtonDown(int button);

	virtual float GetMouseWheelDelta();
	virtual glm::ivec2 GetMousePosition();

protected:
	virtual bool eventFilter(QObject *watched, QEvent *event);

private:
	void onKeyPress(QKeyEvent* e);
	void onMousePress(QMouseEvent* e);
	void onMouseWheel(QWheelEvent* e);

private:
	template <int N>
	struct States {
		enum { Size = N };
		bool pressed[Size] = { false };
		bool up[Size] = { false }, down[Size] = { false };
	};

	QWidget* view_;

	float wheelDelta_ = 0;
	States<3> mouseStates_;
	States<KeyCode::size()> keyStates_;
};
