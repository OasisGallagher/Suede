#pragma once
#include "popupwidget.h"
#include "ui_preferences.h"

class Preferences : public PopupWidget {
	Q_OBJECT

public:
	Preferences(QWidget* parent);

private slots:
	void onSkinChanged(const QString& name);

private:
	void updateUI();

private:
	Ui::Preferences ui_;
};
