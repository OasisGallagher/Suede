#include "colorpicker.h"
#include "math/mathf.h"

#include "tools/enum.h"
#include "memory/memory.h"

static QMetaObject::Connection connection_;

static ColorPicker* colorDialog() {
	static ColorPicker* instance;
	if (instance == nullptr) {
		instance = new ColorPicker;
		instance->setWindowFlags(Qt::Popup);
		instance->setOption(QColorDialog::NoButtons, true);
	}

	return instance;
}

static void showDialog(const QColor& color, QObject* receiver, const char* member) {
	colorDialog()->setOption(QColorDialog::ShowAlphaChannel, true);

	colorDialog()->blockSignals(true);
	colorDialog()->setCurrentColor(color);
	colorDialog()->blockSignals(false);

	connection_ = QObject::connect(colorDialog(), SIGNAL(currentColorChanged(const QColor&)), receiver, member);
	colorDialog()->show();
}

void ColorPicker::display(const Color& color, QObject* receiver, const char* member) {
	Color icolor = color * 255;
	showDialog(QColor(icolor.r, icolor.g, icolor.b), receiver, member);
}

void ColorPicker::destroy() {
	delete colorDialog();
}

void ColorPicker::hideEvent(QHideEvent* event) {
	if (connection_) {
		disconnect(connection_);
	}

	QColorDialog::hideEvent(event);
}

void ColorPicker::mousePressEvent(QMouseEvent* event) {
	QPoint p = event->pos();
	if (p.x() < 0 || p.x() > colorDialog()->width() || p.y() < 0 || p.y() > colorDialog()->height()) {
		colorDialog()->close();
	}
	else {
		pos_ = p;
	}

	QColorDialog::mousePressEvent(event);
}

void ColorPicker::mouseReleaseEvent(QMouseEvent* event) {
	QColorDialog::mouseReleaseEvent(event);
}

void ColorPicker::mouseMoveEvent(QMouseEvent* event) {
	if (event->buttons() & Qt::LeftButton) {
		QPoint diff = event->pos() - pos_;
		colorDialog()->move(colorDialog()->pos() + diff);
	}

	QColorDialog::mouseMoveEvent(event);
}
