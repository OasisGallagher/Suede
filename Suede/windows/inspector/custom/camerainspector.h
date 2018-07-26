#include "rect.h"
#include "custominspector.h"

class CameraInspector : public CustomInspector {
	Q_OBJECT

public:
	CameraInspector(Object object);

private slots:
	void onRectChanged(const Rect& rect);
	void onClearTypeChanged(const QString& text);
	void onClearColorChanged(const QColor& color);
	void onSliderValueChanged(const QString& name, float value);
};
