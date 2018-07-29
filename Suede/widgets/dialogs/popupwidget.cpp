#include "popupwidget.h"

#include <QMouseEvent>

PopupWidget::PopupWidget(QWidget* parent) :QWidget(parent) {
	setWindowFlags(Qt::Popup);
}

void PopupWidget::showEvent(QShowEvent* event) {
	QWidget* p = parentWidget();
	if (p != nullptr) {
		// center this widget.
		move(p->x() + (p->width() - width()) / 2, p->y() + (p->height() - height()) / 2);
	}

	QWidget::showEvent(event);
}

void PopupWidget::mousePressEvent(QMouseEvent* event) {
	QPoint p = event->pos();
	if (p.x() < 0 || p.x() > width() || p.y() < 0 || p.y() > height()) {
		close();
	}
	else {
		pos_ = p;
	}
}

void PopupWidget::mouseMoveEvent(QMouseEvent* event) {
	if (event->buttons() & Qt::LeftButton) {
		QPoint diff = event->pos() - pos_;
		move(pos() + diff);
	}
}
