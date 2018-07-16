#pragma once
#include "vecfield.h"
#include <glm/glm.hpp>

class Vec4Field : public VecField {
	Q_OBJECT

public:
	Vec4Field(QWidget* parent,
		const QStringList& names = QStringList({ "X" , "Y" , "Z" , "W" })
	) : VecField(parent, verifyCount(names, 4)) {}

public:
	void setXRange(float min, float max) { setRange(0, min, max); }
	void setYRange(float min, float max) { setRange(1, min, max); }
	void setZRange(float min, float max) { setRange(2, min, max); }
	void setWRange(float min, float max) { setRange(3, min, max); }

public:
	/**
	 * set value with signal blocked.
	 */
	void setValue(const glm::vec4& value) { setFields((float*)&value); }

signals:
	void valueChanged(const glm::vec4& value);

protected:
	virtual void valueChanged(float* values) {
		emit valueChanged(*((glm::vec4*)values));
	}
};
