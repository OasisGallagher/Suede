#pragma once
#include <QLabel>
#include <QVariant>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QColorDialog>

#include <glm/glm.hpp>

class ColorField : public QWidget {
	Q_OBJECT

public:
	ColorField(QWidget* parent);

public:
	void setValue(const glm::vec4& value);
	void setValue(const glm::vec3& value);

protected:
	virtual void mousePressEvent(QMouseEvent* event);

signals:
	void valueChanged(const QColor& color);

private slots:
	void showColorPicker();
	void onCurrentColorChanged(const QColor& color);

private:
	glm::vec4 color_;

	QLabel* label_;
	QVBoxLayout* layout_;
	QProgressBar* alpha_;
};
