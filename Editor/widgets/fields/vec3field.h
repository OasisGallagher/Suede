#pragma once
#include "vecfield.h"
#include "math/vector3.h"

class Vec3Field : public VecField {
	Q_OBJECT

public:
	Vec3Field(QWidget* parent,
		const QStringList& names = QStringList({ "X" ,"Y" , "Z" })
	) : VecField(parent, verifyCount(names, 3)) {}

public:
	void setXRange(float min, float max) { setRange(0, min, max); }
	void setYRange(float min, float max) { setRange(1, min, max); }
	void setZRange(float min, float max) { setRange(2, min, max); }

public:
	/**
	 * @brief set value with signal blocked.
	 */
	void setValue(const Vector3& value) { setFields((float*)&value); }

signals:
	void valueChanged(const Vector3& value);

protected:
	virtual void valueChanged(float* values) {
		emit valueChanged(*((Vector3*)values));
	}
};
