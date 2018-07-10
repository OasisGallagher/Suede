#include "colorpicker.h"
#include "tools/math2.h"

static QMetaObject::Connection connection_;

static ColorPicker* colorDialog() {
	static ColorPicker* instance;
	if (instance == nullptr) {
		instance = new ColorPicker;
		instance->setWindowFlags(Qt::Popup);
	}

	return instance;
}

static void showDialog(const QColor& color, bool alpha, QObject* receiver, const char* member) {
	colorDialog()->setOption(QColorDialog::ShowAlphaChannel, alpha);
	colorDialog()->setCurrentColor(color);

	connection_ = QObject::connect(colorDialog(), SIGNAL(currentColorChanged(const QColor&)), receiver, member);
	colorDialog()->show();
}

void ColorPicker::display(const glm::vec3& color, QObject* receiver, const char* member) {
	glm::ivec3 ic = Math::IntColor(color);
	showDialog(QColor(ic.r, ic.g, ic.b), false, receiver, member);
}

void ColorPicker::display(const glm::vec4& color, QObject* receiver, const char* member) {
	glm::ivec4 ic = Math::IntColor(color);
	showDialog(QColor(ic.r, ic.g, ic.b, ic.a), true, receiver, member);
}

void ColorPicker::destroy() {
	delete colorDialog();
}

void ColorPicker::hideEvent(QHideEvent* event) {
	disconnect(connection_);
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
}

void ColorPicker::mouseMoveEvent(QMouseEvent* event) {
	if (event->buttons() & Qt::LeftButton) {
		QPoint diff = event->pos() - pos_;
		colorDialog()->move(colorDialog()->pos() + diff);
	}
}
