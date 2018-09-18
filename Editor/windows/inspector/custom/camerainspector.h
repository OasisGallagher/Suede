#include "gui.h"
#include "rect.h"
#include "camera.h"
#include "tools/math2.h"
#include "custominspector.h"

class CameraInspector : public CustomInspector {
public:
	virtual void onGui(Component component) {
		Camera camera = suede_dynamic_cast<Camera>(component);

		int selected = -1;
		if (GUI::EnumPopup("Clear Type", +camera->GetClearType(), selected)) {
			camera->SetClearType(ClearType::value(selected));
		}

		if (camera->GetClearType() == ClearType::Color) {
			Color clearColor = camera->GetClearColor();
			if (GUI::ColorField("Clear Color", clearColor)) {
				camera->SetClearColor(clearColor);
			}
		}

		float fieldOfView = Math::Degrees(camera->GetFieldOfView());
		if (GUI::Slider("FOV", &fieldOfView, 1, 179)) {
			camera->SetFieldOfView(Math::Radians(fieldOfView));
		}

		float nearClipPlane = camera->GetNearClipPlane();
		if (GUI::FloatField("Near", nearClipPlane)) {
			camera->SetNearClipPlane(nearClipPlane);
		}

		float farClipPlane = camera->GetFarClipPlane();
		if (GUI::FloatField("Far", farClipPlane)) {
			camera->SetFarClipPlane(farClipPlane);
		}
	}
};
