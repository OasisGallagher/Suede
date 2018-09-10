#include "preferences.h"

#include <QFile>

#include "main/prefs.h"
#include "main/qtviewer.h"
#include "tools/math2.h"

Preferences::Preferences(QWidget* parent) : PopupWidget(parent) {
	ui_.setupUi(this);

	updateUI();
	connect(ui_.skins, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onSkinChanged(const QString&)));
}

void Preferences::onSkinChanged(const QString& name) {
	QtViewer::setSkin(name);
}

void Preferences::updateUI() {
	QList<QString> skinNames = QtViewer::builtinSkinNames();
	for (QString name : skinNames) {
		ui_.skins->addItem(name);
	}

	ui_.skins->setCurrentText(QtViewer::skinName());
}
