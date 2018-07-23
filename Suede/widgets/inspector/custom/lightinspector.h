#include "custominspector.h"

class LightInspector : public CustomInspector {
	Q_OBJECT

public:
	LightInspector(Object object);

private slots:
	void onIntensityChanged(float intensity);
	void onCurrentColorChanged(const QColor& color);
};
