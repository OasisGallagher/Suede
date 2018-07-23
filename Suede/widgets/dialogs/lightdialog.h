#pragma once
#include "popupwidget.h"
#include "ui_lightdialog.h"

class LightDialog : public PopupWidget {
	Q_OBJECT

public:
	LightDialog(QWidget* parent);
	~LightDialog() {}

protected:
	virtual void showEvent(QShowEvent *event);

private slots:
	void onAmbientChanged(const QColor& color);

private:
	Ui::LightDialog ui_;
};
