#ifdef _DEBUG
#include "vld/vld.h"
#endif

#include <QSurfaceFormat>
#include "qtviewer.h"

void main(int argc, char *argv[]) {
	QSurfaceFormat format;
	format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
// 	format.setRedBufferSize(8);
// 	format.setGreenBufferSize(8);
// 	format.setBlueBufferSize(8);
// 	format.setAlphaBufferSize(8);
// 	format.setDepthBufferSize(24);
// 	format.setStencilBufferSize(8);
	format.setSwapInterval(0);
	QSurfaceFormat::setDefaultFormat(format);

	QtViewer viewer(argc, argv);
	viewer.Run();
}
