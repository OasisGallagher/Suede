#include "custominspector.h"

class CameraInspector : public CustomInspector {
	Q_OBJECT

public:
	CameraInspector(Object object);

private slots:
	void onClearTypeChanged(const QString& text);
	void onSliderValueChanged(const QString& name, float value);
};
