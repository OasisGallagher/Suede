#pragma once
#include <QColorDialog>

class ColorPicker : public QColorDialog {
	Q_OBJECT

public:
	static ColorPicker* get();
	static void destroy();
};
