#ifdef _DEBUG
#include "vld/vld.h"
#endif

#include "qtviewer.h"

#include "light.h"
#include "gameobject.h"

#include "test.h"

typedef std::shared_ptr<int> SharedPtr;

void use(const QList<SharedPtr>& sp) {

}

void call(void* sp) {
	use(*((QList<SharedPtr>*)sp));
}

void main(int argc, char *argv[]) {
	QList<SharedPtr> sharedPtrList({ std::make_shared<int>(0) });
	call(&sharedPtrList);

	QtViewer viewer(argc, argv);
	viewer.Run();
}
