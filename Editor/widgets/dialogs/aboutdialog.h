#pragma once
#include <QDialog>
#include "ui_aboutDialog.h"

class AboutDialog : public QDialog {
	Q_OBJECT

public:
	AboutDialog(QWidget* parent);
	~AboutDialog() {}

public:
	void addInformation(const QString& key, const QString& value);

private slots:
	void onClickCopy();

private:
	QString info_;
	Ui::AboutDialog ui_;
};
