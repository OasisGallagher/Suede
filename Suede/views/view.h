#pragma once
#include <QWidget>

class View : public QWidget {
public:
	void setSize(int w, int h) {
		size_.setWidth(w);
		size_.setHeight(h);
		updateGeometry();
	}

	virtual QSize sizeHint() const {
		return size_;
	}

private:
	QSize size_;
};
