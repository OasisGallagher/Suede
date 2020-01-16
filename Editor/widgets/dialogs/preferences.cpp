#include "preferences.h"

#include "engine.h"
#include "main/prefs.h"
#include "math/mathf.h"
#include "main/qtviewer.h"

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
