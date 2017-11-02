#pragma once
#include <QWidget>
#include "view.h"

class Window : public QObject {
	Q_OBJECT

public:
	Window();

public:
	void setView(View* widget);

protected:
	virtual void initialize() = 0;

protected:
	View* view_;
};
