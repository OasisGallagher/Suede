#include "suede.h"
#include "os/os.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[]) {
	OS::EnableMemoryLeakCheck();

	QApplication a(argc, argv);
	
	QCoreApplication::setOrganizationName("Oasis");
	QCoreApplication::setApplicationName("Suede");

	Suede w;
	w.awake();
	return a.exec();
}
