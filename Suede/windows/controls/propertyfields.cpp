#include <QMatrix>

#include "debug/debug.h"
#include "tools/math2.h"
#include "colorpicker.h"
#include "propertyfields.h"

LabelTexture::LabelTexture(QWidget* parent) : QLabel(parent) {
	setFrameShape(QFrame::Box);
}

void LabelTexture::setColor(const QColor& color) {
	setStyleSheet(QString::asprintf("background-color: rgb(%d,%d,%d)", color.red(), color.green(), color.blue()));
}

void LabelTexture::setColor(const glm::vec3& color) {
	setColor(Math::IntColor(color));
}

void LabelTexture::setColor(const glm::ivec3& color) {
	setColor(QColor(color.r, color.g, color.b));
}

void LabelTexture::setTexture(Texture texture) {
	Texture2D tex = suede_dynamic_cast<Texture2D>(texture);
	if (!tex) { return; }

	std::vector<uchar> data;
	tex->EncodeToPNG(data);

	QPixmap pixmap;
	if (!pixmap.loadFromData(&data[0], data.size())) {
		Debug::LogWarning("failed to load texture");
		return;
	}
	QMatrix matrix;
	float sx = float(size().width()) / pixmap.width(), sy = float(size().height()) / pixmap.height();
	pixmap = pixmap.transformed(matrix.rotate(180).scale(sx, sy));

	setPixmap(pixmap);
}

void LabelTexture::mouseReleaseEvent(QMouseEvent* ev) {
	emit clicked();
}

ColorField::ColorField(QWidget* parent) : QWidget(parent), alpha_(nullptr) {
	layout_ = new QVBoxLayout(this);
	setLayout(layout_);

	label_ = new LabelTexture(this);
	connect(label_, SIGNAL(clicked()), this, SLOT(showColorPicker()));

	layout_->setSpacing(1);
	layout_->setContentsMargins(0, 0, 0, 0);
	layout_->addWidget(label_);
}

void ColorField::setColor(const glm::vec4& color) {
	setColor(glm::vec3(color));

	if (alpha_ == nullptr) {
		QProgressBar* alpha = new QProgressBar(this);
		alpha->setObjectName(alphaObjName());
		alpha->setTextVisible(false);
		alpha->setStyleSheet("QProgressBar::chunk { background-color: #595959 }");
		alpha->setFixedHeight(2);
		alpha->setMaximum(255);
		layout_->addWidget(alpha);
	}

	alpha_->setVisible(true);
	alpha_->setValue(int(color.a* 255));
}

void ColorField::setColor(const glm::vec3& color) {
	color_ = glm::vec4(color, 1);
	label_->setColor(color);
	alpha_->setVisible(false);
}

void ColorField::showColorPicker() {
	ColorPicker::get()->setOption(QColorDialog::ShowAlphaChannel, alpha_ != nullptr && alpha_->isVisible());
	glm::ivec4 color = Math::IntColor(color_);
	QColor old(color.r, color.g, color.b, color.a);

	ColorPicker::get()->blockSignals(true);
	ColorPicker::get()->setCurrentColor(old);
	ColorPicker::get()->blockSignals(false);
	
	ColorPicker::get()->setProperty("ColorField_Prop", data_);
	ColorPicker::popup();
}

void ColorField::onColorPickerVisible(bool visible) {
	if (visible) {
		connect(ColorPicker::get(), SIGNAL(currentColorChanged(const QColor&)), this, SLOT(onColorPicked(const QColor&)));
	}
	else {
		disconnect(ColorPicker::get(), SIGNAL(currentColorChanged(const QColor&)), this, SLOT(onColorPicked(const QColor&)));
	}
}

void ColorField::onColorPicked(const QColor& color) {
	emit currentColorChanged(color);
}
