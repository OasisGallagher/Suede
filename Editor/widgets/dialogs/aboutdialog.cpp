#include <QClipboard>

#include "aboutDialog.h"

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent) {
	ui_.setupUi(this);
	setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
	connect(ui_.close, SIGNAL(clicked()), this, SLOT(accept()));
	connect(ui_.copy, SIGNAL(clicked()), this, SLOT(onClickCopy()));
}

void AboutDialog::addInformation(const QString& key, const QString& value) {
	ui_.formLayout->addRow(key, new QLabel(value));
	if (!info_.isEmpty()) { info_ += "\n"; }
	info_ += (key + "\t" + value);
}

void AboutDialog::onClickCopy() {
	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setText(info_);
}
