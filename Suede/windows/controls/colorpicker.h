#pragma once
#include <QColorDialog>

class ColorListener {
public:
	virtual void onColorChanged(const QColor& color) = 0;
};

class ColorPicker : public QColorDialog {
	Q_OBJECT

public:
	ColorPicker();

public:
	static ColorPicker* get();

	static void popup();
	static void destroy();

public:
	void setColorListener(ColorListener* value) { listener_ = value; }

private slots :
	void onColorChanged(const QColor& color);

private:
	ColorListener* listener_;
};
