#include "custominspector.h"

class MeshInspector : public CustomInspector {
	Q_OBJECT

public:
	MeshInspector(Object object);

private slots:
	void onTextChanged();
};
