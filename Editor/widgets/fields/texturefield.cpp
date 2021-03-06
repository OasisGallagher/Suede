#include <QMouseEvent>
#include <QFileDialog>

#include "resources.h"
#include "texturefield.h"

#include "debug/debug.h"

static QMetaObject::Connection connection_;

TextureField::TextureField(QWidget* parent) : QWidget(parent) {
	label_ = new QLabel(this);
	setFixedSize(32, 32);
}

void TextureField::setTexture(Texture* value) {
	Texture2D* tex = (Texture2D*)value;
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

	label_->setPixmap(pixmap);
}

void TextureField::mousePressEvent(QMouseEvent* event) {
	showTextureExplorer();
}

void TextureField::showTextureExplorer() {
	if (connection_) {
		disconnect(connection_);
	}

	QString path = QFileDialog::getOpenFileName(this, "Select Texture*", Resources::textureDirectory, "*.jpg;;*.png");
	if (!path.isEmpty()) {
		ref_ptr<Texture2D> texture = new Texture2D();
		path = QDir(Resources::textureDirectory).relativeFilePath(path);

		if (texture->Load(path.toStdString())) {
			setTexture(texture.get());
			emit currentTextureChanged(texture.get());
		}
	}
}
