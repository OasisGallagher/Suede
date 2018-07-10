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
	void setColor(const glm::vec4& value);
	void setColor(const glm::vec3& value);

	void setData(const QVariant& data) { data_ = data; }
	const QVariant& data() const { return data_; }

protected:
	virtual void mousePressEvent(QMouseEvent* event);

signals:
	void currentColorChanged(const QColor& color);

private slots:
	void showColorPicker();
	void onCurrentColorChanged(const QColor& color);

private:
	QVariant data_;
	glm::vec4 color_;

	QLabel* label_;
	QVBoxLayout* layout_;
	QProgressBar* alpha_;
};
