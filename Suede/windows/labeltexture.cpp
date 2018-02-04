#include <QMatrix>

#include "debug/debug.h"
#include "tools/math2.h"
#include "labeltexture.h"

LabelTexture::LabelTexture(QWidget* parent) : QLabel(parent) {
}

void LabelTexture::setColor(const QColor& color) {
	setStyleSheet(QString::asprintf("border: 0; background-color: rgb(%d,%d,%d)", color.red(), color.green(), color.blue()));
}

void LabelTexture::setColor(const glm::vec4& color) {
	setColor(Math::IntColor(color));
}

void LabelTexture::setColor(const glm::ivec4& color) {
	setColor(QColor(color.r, color.g, color.b));
}

void LabelTexture::setTexture(Texture texture) {
	Texture2D tex = dsp_cast<Texture2D>(texture);
	if (!tex) { return; }

	std::vector<uchar> data;
	tex->EncodeToPng(data);

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

void LabelTexture::mouseReleaseEvent(QMouseEvent * ev) {
	emit clicked();
}
