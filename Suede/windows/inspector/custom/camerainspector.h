#include "custominspector.h"

class CameraInspector : public CustomInspector {
	Q_OBJECT

public:
	CameraInspector(Object object);

private slots:
	void onSliderValueChanged(const QString& name, float value);
};
