#include "custominspector.h"

class MeshInspector : public CustomInspector {
	Q_OBJECT

public:
	MeshInspector(Object object);

private:
	void drawMesh();
	void drawTextMesh();

private slots:
	void onTextChanged();
};
