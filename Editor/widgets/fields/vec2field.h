#pragma once
#include "vecfield.h"
#include "math/vector2.h"

class Vec2Field : public VecField {
	Q_OBJECT

public:
	Vec2Field(QWidget* parent,
		const QStringList& names = QStringList({ "X", "Y" })
	) : VecField(parent, verifyCount(names, 2)) {}

public:
	void setXRange(float min, float max) { setRange(0, min, max); }
	void setYRange(float min, float max) { setRange(1, min, max); }

public:
	/**
	 * @brief set value with signal blocked.
	 */
	void setValue(const Vector2& value) { setFields((float*)&value); }

signals:
	void valueChanged(const Vector2& value);

protected:
	virtual void valueChanged(float* values) {
		emit valueChanged(*((Vector2*)values));
	}
};
