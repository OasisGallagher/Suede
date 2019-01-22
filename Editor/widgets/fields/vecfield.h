#pragma once
#include <QWidget>

class FloatField;
class VecField : public QWidget {
	Q_OBJECT

public:
	VecField(QWidget* parent, const QStringList& names);
	~VecField();

public:
	/**
	 * set values with signal blocked.
	 */
	void setFields(float* values);
	void setRange(uint index, float min, uint max);

protected:
	virtual void valueChanged(float* values) = 0;

protected:
	const QStringList& verifyCount(const QStringList& names, uint count);

private slots:
	void onFieldValueChanged();

private:
	uint count_;
	FloatField** fields_;
};
