#pragma once
#include <QLineEdit>

class IntField : public QLineEdit {
	Q_OBJECT

public:
	IntField(QWidget* parent);

public:
	int value() const { return value_; }
	void setValue(int value);

	void setRange(int min, int max) { min_ = min, max_ = max; }

signals:
	void valueChanged(int value);

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

	int step_;
	int value_;
	int min_, max_;

	bool dragging_;
};
