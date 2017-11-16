#include "suede.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[]) {
	// TODO: platform.
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);

	QApplication a(argc, argv);
	Suede w;
	w.show();
	return a.exec();
}
