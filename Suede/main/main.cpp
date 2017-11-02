#include "suede.h"
#include <QtWidgets/QApplication>
#include <memory>

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	Suede w;
	w.show();
	return a.exec();
}
