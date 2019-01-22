#pragma once
#include <QSlider>

class FloatField;
class RangeField : public QWidget {
	Q_OBJECT

public:
	RangeField(QWidget* parent = Q_NULLPTR);

public:
	void setRange(float min, float max);

	float value() const;
	void setValue(float x);

signals:
	void valueChanged(float value);

private slots:
	void onEditValue(float value);
	void onSlideValue(int value);

protected:
	virtual bool eventFilter(QObject *watched, QEvent *event);

private:
	QSlider* slider_;
	FloatField* edit_;
	float min_, max_;
};
