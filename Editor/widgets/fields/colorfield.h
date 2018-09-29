#pragma once
#include <QLabel>
#include <QVBoxLayout>
#include <QProgressBar>

#include <glm/glm.hpp>

#include "color.h"

class ColorField : public QWidget {
	Q_OBJECT

public:
	ColorField(QWidget* parent);

public:
	void setValue(const Color& value);

protected:
	virtual void mousePressEvent(QMouseEvent* event);

signals:
	void valueChanged(const QColor& color);

private slots:
	void showColorPicker();
	void onCurrentColorChanged(const QColor& color);

private:
	Color color_;

	QLabel* label_;
	QVBoxLayout* layout_;
	QProgressBar* alpha_;
};
