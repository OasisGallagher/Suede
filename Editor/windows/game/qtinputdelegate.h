#pragma once
#include "input.h"
#include <QWidget>
#include <QMouseEvent>

class QtInputDelegate : public QObject, public InputDelegate {
	Q_OBJECT

public:
	QtInputDelegate(QWidget* view);
	~QtInputDelegate() {}

public:
	virtual Vector2 GetMousePosition();

protected:
	virtual bool eventFilter(QObject *watched, QEvent *event);

private:
	void onKeyPress(QKeyEvent* e);
	void onMousePress(QMouseEvent* e);
	void onMouseWheel(QWheelEvent* e);

private:
	QWidget* view_;
};
