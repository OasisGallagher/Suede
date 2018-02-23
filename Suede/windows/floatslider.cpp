#include <QEvent>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QCoreApplication>

#include "floatslider.h"

class Slider : public QSlider {
public:
	Slider(QWidget *parent = Q_NULLPTR) : QSlider(parent) {
		setOrientation(Qt::Horizontal);
	}

protected:
	virtual void mousePressEvent(QMouseEvent* ev);
};

FloatSlider::FloatSlider(QWidget* parent) : QWidget(parent) {
	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setMargin(0);

	label_ = new QLabel(this);
	slider_ = new Slider(this);
	connect(slider_, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));

	layout->addWidget(slider_);
	layout->addWidget(label_);
}

void FloatSlider::setRange(uint min, uint max) {
	slider_->setRange(min * 100, max * 100);
}

float FloatSlider::value() const {
	return slider_->value() / 100.f;
}

void FloatSlider::setValue(float x) {
	int ux = x * 100;
	slider_->setValue(ux);
	label_->setText(QString::number(x, 'f', 2));
}

void FloatSlider::onSliderValueChanged(int value) {
	label_->setText(QString::number(value / 100.f, 'f', 2));
	emit valueChanged(objectName(), value / 100.f);
}

void Slider::mousePressEvent(QMouseEvent* ev) {
	QSlider::mousePressEvent(ev);

	float pos = ev->pos().x() / (float)width();
	setValue(pos * (maximum() - minimum()) + minimum());
}
