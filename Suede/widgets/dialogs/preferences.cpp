#include "preferences.h"

#include <QFile>

#include "main/prefs.h"
#include "tools/math2.h"

Preferences::Preferences(QWidget* parent) : PopupWidget(parent) {
	ui_.setupUi(this);

	updateUI();
	connect(ui_.skins, SIGNAL(currentIndexChanged(int)), this, SLOT(onSkinChanged(int)));
}

void Preferences::onSkinChanged(int index) {
	QString qss;
	if (!builtinSkins[index].qssFile.isEmpty()) {
		QFile file(":/qss/style");
		file.open(QFile::ReadOnly);
		qss = file.readAll();
		file.close();
	}

	qApp->setStyleSheet(qss);
	Prefs::get()->save("skin", builtinSkins[index].name);
}

void Preferences::updateUI() {
	int index = 0;
	QString savedSkin = Prefs::get()->load("skin", builtinSkins[0].name).toString();
	for (int i = 0; i < CountOf(builtinSkins); ++i) {
		ui_.skins->addItem(builtinSkins[i].name);
		if (builtinSkins[i].name == savedSkin) {
			index = i;
		}
	}

	ui_.skins->setCurrentIndex(index);
}
