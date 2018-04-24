#include <QMatrix>

#include "debug/debug.h"
#include "tools/math2.h"
#include "labeltexture.h"

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

void LabelTexture::mouseReleaseEvent(QMouseEvent * ev) {
	emit clicked();
}
