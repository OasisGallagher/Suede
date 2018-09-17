#include "gui.h"
#include "light.h"
#include "custominspector.h"

class LightInspector : public CustomInspector {
public:
	virtual void onGui(Component component) {
		Light light = suede_dynamic_cast<Light>(component);
		glm::vec3 color = light->GetColor();
		if (GUI::Color3Field("Color", color)) {
			light->SetColor(color);
		}

		float intensity = light->GetIntensity();
		if (GUI::FloatField("Intensity", intensity)) {
			light->SetIntensity(intensity);
		}
	}
};
