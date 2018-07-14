#pragma once
#include <QWidget>

class FloatField;
class VecField : public QWidget {
	Q_OBJECT

public:
	VecField(QWidget* parent, const QStringList& names);
	~VecField();

public:
	void setFields(float* values);

protected:
	virtual void valueChanged(float* values) = 0;

protected:
	const QStringList& verifyCount(const QStringList& names, uint count);

private slots:
	void onFieldValueChanged(float value);

private:
	uint count_;
	FloatField** fields_;
};
