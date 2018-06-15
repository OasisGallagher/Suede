#include <QFile>
#include <QSplashScreen>

#include "suede.h"
#include "engine.h"
#include "qtviewer.h"

QtViewer::QtViewer(int argc, char * argv[]) : GraphicsViewer(argc, argv), app_(argc, argv) {
	QSplashScreen* splash = new QSplashScreen;
	splash->setPixmap(QPixmap(":/images/splash"));
	splash->show();

	app_.processEvents();

	setup();

	Suede suede;
	Canvas* c = Game::get()->canvas();
	connect(c, SIGNAL(sizeChanged(uint, uint)), this, SLOT(canvasSizeChanged(uint, uint)));

	SetCanvas(c);
	_GraphicsViewer_();
	suede.awake();

	splash->finish(&suede);
	delete splash;

}

void QtViewer::Update() {
	GraphicsViewer::Update();
	app_.processEvents();
}

void QtViewer::setup() {
	QFile qss(":/qss/style");
	qss.open(QFile::ReadOnly);
	QString str = qss.readAll();
	qApp->setStyleSheet(str);
	qss.close();

	QCoreApplication::setOrganizationName("Oasis");
	QCoreApplication::setApplicationName("Suede");
}
