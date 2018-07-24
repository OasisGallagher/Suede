#include <QMouseEvent>

#include "colorfield.h"
#include "../widgets/dialogs/colorpicker.h"

#include "debug/debug.h"
#include "tools/math2.h"

ColorField::ColorField(QWidget* parent) : QWidget(parent), alpha_(nullptr) {
	layout_ = new QVBoxLayout(this);
	setLayout(layout_);
	layout_->setSpacing(1);
	layout_->setContentsMargins(0, 0, 0, 0);

	label_ = new QLabel(this);
	layout_->addWidget(label_);
}

void ColorField::setValue(const glm::vec4& value) {
	setValue(glm::vec3(value));

	if (alpha_ == nullptr) {
		alpha_ = new QProgressBar(this);
		alpha_->setTextVisible(false);
		alpha_->setStyleSheet("QProgressBar::chunk { background-color: #595959 }");
		alpha_->setFixedHeight(2);
		alpha_->setMaximum(255);
		layout_->addWidget(alpha_);
	}

	alpha_->setVisible(true);
	alpha_->setValue(int(value.a* 255));

	color_ = value;
}

void ColorField::setValue(const glm::vec3& value) {
	color_ = glm::vec4(value, 1);

	glm::ivec3 icolor = Math::IntColor(value);
	label_->setStyleSheet(QString::asprintf("background-color: rgb(%d,%d,%d)", icolor.r, icolor.g, icolor.b));

	if (alpha_ != nullptr) {
		alpha_->setVisible(false);
	}
}

void ColorField::mousePressEvent(QMouseEvent* event) {
	showColorPicker();
}

void ColorField::showColorPicker() {
	if (alpha_ != nullptr && alpha_->isVisible()) {
		ColorPicker::display(color_, this, SLOT(onCurrentColorChanged(const QColor&)));
	}
	else {
		ColorPicker::display(glm::vec3(color_), this, SLOT(onCurrentColorChanged(const QColor&)));
	}
}

void ColorField::onCurrentColorChanged(const QColor& color) {
	if (alpha_ != nullptr && alpha_->isVisible()) {
		setValue(glm::vec4(color.red(), color.green(), color.blue(), color.alpha()) / 255.f);
		alpha_->setValue(color.alpha());
	}
	else {
		setValue(glm::vec3(color.red(), color.green(), color.blue()) / 255.f);
	}

	emit valueChanged(color);
}
