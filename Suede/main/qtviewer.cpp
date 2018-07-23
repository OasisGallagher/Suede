#include <QFile>
#include <QSplashScreen>

#include "suede.h"
#include "qtviewer.h"

QtViewer::QtViewer(int argc, char * argv[]) : GraphicsViewer(argc, argv), app_(argc, argv) {
	//QSplashScreen* splash = new QSplashScreen;
	//splash->setPixmap(QPixmap(":/images/splash"));
	//splash->show();

	//app_.processEvents();

	setupRegistry();
	setupStyle();
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

void QtViewer::setupStyle() {
	QFile qss(":/qss/style");
	qss.open(QFile::ReadOnly);
	QString str = qss.readAll();
	qApp->setStyleSheet(str);
	qss.close();
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
