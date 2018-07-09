#pragma once
#include <QLabel>
#include <QVariant>
#include <QVBoxLayout>
#include <QProgressBar>

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

class ColorField : public QWidget {
public:
	static QString alphaObjName() { return "alpha"; }

public:
	ColorField(QWidget* parent);

public:
	void setColor(const glm::vec4& color);
	void setColor(const glm::vec3& color);

	void setData(const QVariant& data) { data_ = data; }
	const QVariant& data() const { return data_; }

signals:
	void currentColorChanged(const QColor& color);

private slots:
	void showColorPicker();

	void onColorPickerVisible(bool visible);
	void onColorPicked(const QColor& color);

private:
	QVariant data_;
	glm::vec4 color_;

	QVBoxLayout* layout_;
	LabelTexture* label_;
	QProgressBar* alpha_;
};
