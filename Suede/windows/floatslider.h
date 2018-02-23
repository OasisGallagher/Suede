#pragma once
#include <QLabel>
#include <QSlider>
#include <QWidget>

class FloatSlider : public QWidget {
	Q_OBJECT

public:
	FloatSlider(QWidget* parent = Q_NULLPTR);

public:
	void setRange(uint min, uint max);

	float value() const;
	void setValue(float x);

signals:
	void valueChanged(const QString& name, float value);

private slots:
	void onSliderValueChanged(int value);

private:
	QLabel* label_;
	QSlider* slider_;
};
