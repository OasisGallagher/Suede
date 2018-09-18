#include "gui.h"
#include "light.h"
#include "custominspector.h"

class LightInspector : public CustomInspector {
public:
	virtual void onGui(Component component) {
		Light light = suede_dynamic_cast<Light>(component);
		Color color = light->GetColor();
		if (GUI::ColorField("Color", color)) {
			light->SetColor(color);
		}

		float intensity = light->GetIntensity();
		if (GUI::FloatField("Intensity", intensity)) {
			light->SetIntensity(intensity);
		}
	}
};
