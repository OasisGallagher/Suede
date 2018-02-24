#include "custominspector.h"

class ProjectorInspector : public CustomInspector {
	Q_OBJECT

public:
	ProjectorInspector(Object object);

private slots:
	void onSliderValueChanged(const QString& name, float value);
};
