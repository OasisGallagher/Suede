#pragma once
#include <QLabel>
#include <QLineEdit>

class FloatLabel : public QLabel {
	Q_OBJECT

public:
	FloatLabel(QWidget* parent);

public:
	float step() const { return step_; }
	void setStep(float value) { step_ = value; }

	uint precision() const { return precision_; }
	void setPrecision(uint value);

protected:
	virtual void mousePressEvent(QMouseEvent *ev);
	virtual void mouseReleaseEvent(QMouseEvent *ev);

	virtual void mouseMoveEvent(QMouseEvent *ev);

private:
	void updateText(float f);

private:
	float step_;
	float float_;

	bool dragging_;
	uint precision_;

	QPoint pos_;
};
