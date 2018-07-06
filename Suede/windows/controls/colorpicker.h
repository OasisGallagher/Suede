#pragma once
#include <QColorDialog>

class ColorPicker : public QColorDialog {
	Q_OBJECT

public:
	ColorPicker();

public:
	static ColorPicker* get();

	static void popup();
	static void destroy();

protected:
	virtual void focusOutEvent(QFocusEvent* event);
};
