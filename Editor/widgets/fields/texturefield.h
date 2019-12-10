#pragma once
#include <QLabel>
#include <QWidget>
#include <QVariant>

#include "texture.h"

class TextureField : public QWidget {
	Q_OBJECT

public:
	TextureField(QWidget* parent);

public:
	void setTexture(Texture value);

	void setData(const QVariant& data) { data_ = data; }
	const QVariant& data() const { return data_; }

protected:
	virtual void mousePressEvent(QMouseEvent* event);

signals:
	void currentTextureChanged(Texture texture);

private slots:
	void showTextureExplorer();

private:
	QLabel* label_;
	QVariant data_;
};
