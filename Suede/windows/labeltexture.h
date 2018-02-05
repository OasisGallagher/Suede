#pragma once
#include <QLabel>
#include <glm/glm.hpp>

#include "texture.h"

class LabelTexture : public QLabel {
	Q_OBJECT
public:
	LabelTexture(QWidget* parent = Q_NULLPTR);

public:
	void setColor(const QColor& color);
	void setColor(const glm::vec3& color);
	void setColor(const glm::ivec3& color);

	void setTexture(Texture texture);

signals:
	void clicked();

protected:
	virtual void mouseReleaseEvent(QMouseEvent *ev);
};