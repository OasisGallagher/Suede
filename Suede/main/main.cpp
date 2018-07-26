#ifdef _DEBUG
#include "vld/vld.h"
#endif

#include <QtOpenGL/QGLFormat>
#include "qtviewer.h"

#define D(x, y)	QString(#y)

void main(int argc, char *argv[]) {
	QtViewer viewer(argc, argv);
	viewer.Run();
}
