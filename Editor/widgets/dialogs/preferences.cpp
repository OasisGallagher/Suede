#include "preferences.h"

#include <QFile>

#include "physics.h"
#include "main/prefs.h"
#include "math/mathf.h"
#include "main/qtviewer.h"

Preferences::Preferences(QWidget* parent) : PopupWidget(parent) {
	ui_.setupUi(this);

	updateUI();
	connect(ui_.skins, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onSkinChanged(const QString&)));
	connect(ui_.physicsDebugDrawEnabled, SIGNAL(stateChanged(int)), this, SLOT(onPhysicsDebugDrawEnabledChanged(int)));
}

void Preferences::onSkinChanged(const QString& name) {
	QtViewer::setSkin(name);
}

void Preferences::onPhysicsDebugDrawEnabledChanged(int state) {
	Physics::SetDebugDrawEnabled(!!state);
}

void Preferences::updateUI() {
	QList<QString> skinNames = QtViewer::builtinSkinNames();
	for (QString name : skinNames) {
		ui_.skins->addItem(name);
	}

	ui_.skins->setCurrentText(QtViewer::skinName());
	ui_.physicsDebugDrawEnabled->setChecked(Physics::GetDebugDrawEnabled());
}
