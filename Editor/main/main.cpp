#ifdef _DEBUG
#include "vld/vld.h"
#endif

#include "qtviewer.h"

#include "light.h"
#include "gameobject.h"

#include "test.h"

void main(int argc, char *argv[]) {
	QtViewer viewer(argc, argv);
	viewer.Run();
}
