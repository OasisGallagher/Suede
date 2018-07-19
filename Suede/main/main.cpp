#ifdef _DEBUG
#include "vld/vld.h"
#endif

#include <QtOpenGL/QGLFormat>
#include "qtviewer.h"

void main(int argc, char *argv[]) {
	QtViewer viewer(argc, argv);
	viewer.Run();
}
