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
	void setValue(const glm::vec4& value) { setFields((float*)&value); }

signals:
	void valueChanged(const glm::vec4& value);

protected:
	virtual void valueChanged(float* values) {
		emit valueChanged(*((glm::vec4*)values));
	}
};
