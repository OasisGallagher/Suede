#pragma once
#include <QHideEvent>
#include <QMouseEvent>
#include <QColorDialog>

#include <glm/glm.hpp>

class ColorPicker : public QColorDialog {
	Q_OBJECT

public:
	static void display(const glm::vec3& color, QObject* receiver, const char* member);
	static void display(const glm::vec4& color, QObject* receiver, const char* member);
	
	static void destroy();

signals:
	void currentColorChanged(const QColor& color);

protected:
	virtual void hideEvent(QHideEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);

private:
	QPoint pos_;
	QMetaObject::Connection connection_;
};
