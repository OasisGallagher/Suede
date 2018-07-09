#pragma once
#include <QColorDialog>

class ColorPicker : public QColorDialog {
	Q_OBJECT

public:
	ColorPicker();

protected:
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void focusOutEvent(QFocusEvent* event);

public:
	static ColorPicker* get();

	static void popup();
	static void destroy();

private:
	QPoint pos_;
};
