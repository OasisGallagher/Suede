#pragma once
#include <QLineEdit>

class FloatField : public QLineEdit {
	Q_OBJECT

public:
	FloatField(QWidget* parent);

public:
	float value() const { return value_; }
	void setValue(float value);

	uint precision() const { return precision_; }
	void setPrecision(uint value) { precision_ = value; }

	void setRange(float min, float max) { min_ = min, max_ = max; }

signals:
	void valueChanged(float value);

protected:
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseReleaseEvent(QMouseEvent* e);

private slots:
	void onEditingFinished();

private:
	void updateText();

private:
	QPoint pos_;

	float step_;
	float value_;
	float min_, max_;

	bool dragging_;

	uint precision_;
};
