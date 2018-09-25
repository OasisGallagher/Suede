#pragma once
#include "popupwidget.h"
#include "ui_shaderselector.h"

class ShaderSelector : public QDialog {
	Q_OBJECT

public:
	ShaderSelector(QWidget* parent);

public:
	QString select(const QString& current);

private slots:
	void onSelectShader(const QString& path);

private:
	QString selected_;
	Ui::ShaderSelector ui;
};
