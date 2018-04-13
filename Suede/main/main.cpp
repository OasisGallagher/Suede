#ifdef _DEBUG
#include "vld/vld.h"
#endif
#include <QFile>
#include <QSplashScreen>

#include "suede.h"
#include "os/filesystem.h"
#include <QtWidgets/QApplication>

static void setup() {
	QFile qss(":/qss/style");
	qss.open(QFile::ReadOnly);
	QString str = qss.readAll();
	qApp->setStyleSheet(str);
	qss.close();

	QCoreApplication::setOrganizationName("Oasis");
	QCoreApplication::setApplicationName("Suede");
}

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	
	QSplashScreen* splash = new QSplashScreen;
	splash->setPixmap(QPixmap(":/images/splash"));
	splash->show();

	a.processEvents();

	setup();

	Suede w;
	w.awake();
	
	splash->finish(&w);
	delete splash;

	return a.exec();
}
