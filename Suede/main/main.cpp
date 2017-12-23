#include "suede.h"
#include "os/os.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[]) {
	OS::EnableMemoryLeakCheck();

	QApplication a(argc, argv);
	Suede w;
	w.show();
	return a.exec();
}
