#include "qtviewer.h"

#include <QFile>
#include <QSplashScreen>

#include "prefs.h"
#include "editor.h"

QString QtViewer::s_defaultSkin("Default");

QMap<QString, QString> QtViewer::s_skinResources({
	std::make_pair(s_defaultSkin, ""),
	std::make_pair("Dark",":/qss/style"),
});

QtViewer::QtViewer(int argc, char * argv[]) : GraphicsViewer(argc, argv), app_(argc, argv) {
// 	QSplashScreen* splash = new QSplashScreen;
// 	splash->setPixmap(QPixmap(":/images/splash"));
// 	splash->show();

	app_.processEvents();

	setupRegistry();

	setSkin(skinName());
	setupEditor();

// 	splash->finish(editor_);
// 	delete splash;
}

QtViewer::~QtViewer() {
	delete editor_;
}

void QtViewer::Update() {
	app_.processEvents();
	editor_->tick();
}

QList<QString> QtViewer::builtinSkinNames() {
	return s_skinResources.keys();
}

QString QtViewer::skinName() {
	return Prefs::instance()->load("skin", s_defaultSkin).toString();
}

bool QtViewer::setSkin(const QString& name) {
	QMap<QString, QString>::iterator ite = s_skinResources.find(name);
	if (ite == s_skinResources.end()) {
		Debug::LogError(("invalid skin name " + name).toLatin1().data());
		return false;
	}

	QString qss;
	if (!ite.value().isEmpty()) {
		QFile file(":/qss/style");
		file.open(QFile::ReadOnly);
		qss = file.readAll();
		file.close();
	}

	qApp->setStyleSheet(qss);
	Prefs::instance()->save("skin", name);

	return true;
}

void QtViewer::setupEditor() {
	QGLFormat format;
	format.setSwapInterval(1);
	format.setDoubleBuffer(true);
	format.setSampleBuffers(true);

	QGLFormat::setDefaultFormat(format);

	editor_ = new Editor();
	connect(editor_, SIGNAL(aboutToClose()), this, SLOT(onAboutToCloseEditor()));

	Canvas* c = Game::instance()->canvas();
	connect(c, SIGNAL(sizeChanged(uint, uint)), this, SLOT(canvasSizeChanged(uint, uint)));

	editor_->init();

	if (SetCanvas(c)) {
		editor_->awake();
	}

	editor_->show();
}

void QtViewer::setupRegistry() {
	QCoreApplication::setOrganizationName("Oasis");
	QCoreApplication::setApplicationName("Suede");
}
