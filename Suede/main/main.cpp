#ifdef _DEBUG
#include "vld/vld.h"
#endif

#include "suede.h"
#include "os/filesystem.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	
	QCoreApplication::setOrganizationName("Oasis");
	QCoreApplication::setApplicationName("Suede");

	Suede w;
	w.awake();
	return a.exec();
}
