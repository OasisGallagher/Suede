#pragma once
#include "rect.h"
#include "vecfield.h"

class RectField : public VecField {
	Q_OBJECT

public:
	RectField(QWidget* parent)
		: VecField(parent, QStringList({ "X", "Y", "W", "H" })) {
		for (int i = 0; i < 4; ++i) { setRange(i, 0, 1); }
	}

public:
	/**
	 * @brief set rect with signal blocked.
	 */
	void setValue(const Rect& value) { setFields((float*)&value); }

signals:
	void valueChanged(const Rect& value);

protected:
	virtual void valueChanged(float* values) {
		emit valueChanged(*((Rect*)values));
	}
};
