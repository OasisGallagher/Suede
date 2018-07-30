#include "qtviewer.h"

#include <QFile>
#include <QSplashScreen>

#include "prefs.h"
#include "suede.h"

QMap<QString, QString> QtViewer::skinResources({
	std::make_pair("Default", ""),
	std::make_pair("Dark",":/qss/style"),
});

QtViewer::QtViewer(int argc, char * argv[]) : GraphicsViewer(argc, argv), app_(argc, argv) {
	//QSplashScreen* splash = new QSplashScreen;
	//splash->setPixmap(QPixmap(":/images/splash"));
	//splash->show();

	//app_.processEvents();

	setupRegistry();

	setSkin(skinName());
	setupSuede();

	//splash->finish(suede_);
	//delete splash;
}

QtViewer::~QtViewer() {
	delete suede_;
}

void QtViewer::Update() {
	GraphicsViewer::Update();
	app_.processEvents();
}

QList<QString> QtViewer::builtinSkinNames() {
	return skinResources.keys();
}

QString QtViewer::skinName() {
	return Prefs::get()->load("skin").toString();
}

bool QtViewer::setSkin(const QString& name) {
	QMap<QString, QString>::iterator ite = skinResources.find(name);
	if (ite == skinResources.end()) {
		Debug::LogError(("invalid skin name " + name).toLatin1());
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
	Prefs::get()->save("skin", name);

	return true;
}

void QtViewer::setupSuede() {
	QGLFormat format;
	format.setSwapInterval(0);
	format.setDoubleBuffer(true);
	format.setSampleBuffers(true);

	QGLFormat::setDefaultFormat(format);

	suede_ = new Suede();
	connect(suede_, SIGNAL(aboutToClose()), this, SLOT(onAboutToCloseSuede()));

	Canvas* c = Game::get()->canvas();
	connect(c, SIGNAL(sizeChanged(uint, uint)), this, SLOT(canvasSizeChanged(uint, uint)));

	SetCanvas(c);
	suede_->awake();
}

void QtViewer::setupRegistry() {
	QCoreApplication::setOrganizationName("Oasis");
	QCoreApplication::setApplicationName("Suede");
}
