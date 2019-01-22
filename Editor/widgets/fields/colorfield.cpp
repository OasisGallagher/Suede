#include <QMouseEvent>

#include "colorfield.h"
#include "../dialogs/colorpicker.h"

ColorField::ColorField(QWidget* parent) : QWidget(parent) {
	layout_ = new QVBoxLayout(this);
	setLayout(layout_);
	layout_->setSpacing(1);
	layout_->setContentsMargins(0, 0, 0, 0);

	label_ = new QLabel(this);
	layout_->addWidget(label_);

	alpha_ = new QProgressBar(this);
	layout_->addWidget(alpha_);

	alpha_->setTextVisible(false);
	alpha_->setStyleSheet("QProgressBar::chunk { background-color: #595959 }");
	alpha_->setFixedHeight(2);
	alpha_->setMaximum(255);
}

void ColorField::setValue(const Color& value) {
	color_ = value;

	Color icolor = value * 255;
	label_->setStyleSheet(QString::asprintf("background-color: rgb(%d,%d,%d)", int(icolor.r), int(icolor.g), (int)icolor.b));
	alpha_->setValue(icolor.a);
}

void ColorField::mousePressEvent(QMouseEvent* event) {
	showColorPicker();
}

void ColorField::showColorPicker() {
	ColorPicker::display(color_, this, SLOT(onCurrentColorChanged(const QColor&)));
}

void ColorField::onCurrentColorChanged(const QColor& color) {
	setValue(Color(color.redF(), color.greenF(), color.blueF(), color.alphaF()));
	emit valueChanged(color);
}
