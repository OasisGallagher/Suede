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
	for (int i = 0; i < skinNames.count(); ++i) {
		ui_.skins->addItem(skinNames[i]);
	}

	ui_.skins->setCurrentText(QtViewer::skinName());
}
