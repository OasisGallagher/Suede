#include "gui.h"
#include "light.h"
#include "custominspector.h"

class LightInspector : public CustomInspectorT<Light> {
public:
	virtual void onGui() {
		glm::vec3 color = target_->GetColor();
		if (GUI::Color3Field("Color", color)) {
			target_->SetColor(color);
		}

		float intensity = target_->GetIntensity();
		if (GUI::FloatField("Intensity", intensity)) {
			target_->SetIntensity(intensity);
		}
	}
};
