#pragma once
#include <QWidget>

class PopupWidget : public QWidget {
	Q_OBJECT

public:
	PopupWidget(QWidget* parent);

protected:
	virtual void showEvent(QShowEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);

private:
	QPoint pos_;
};