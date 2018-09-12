#include "gui.h"
#include "rect.h"
#include "camera.h"
#include "custominspector.h"

class CameraInspector : public CustomInspectorT<Camera> {
public:
	virtual void onGui() {
		int selected = -1;
		if (GUI::EnumPopup("Clear Type", +target_->GetClearType(), selected)) {
			target_->SetClearType(ClearType::value(selected));
		}

		if (target_->GetClearType() == ClearType::Color) {
			glm::vec3 clearColor = target_->GetClearColor();
			if (GUI::Color3Field("Clear Color", clearColor)) {
				target_->SetClearColor(clearColor);
			}
		}

		float fieldOfView = Math::Degrees(target_->GetFieldOfView());
		if (GUI::Slider("FOV", &fieldOfView, 1, 179)) {
			target_->SetFieldOfView(Math::Radians(fieldOfView));
		}

		float nearClipPlane = target_->GetNearClipPlane();
		if (GUI::FloatField("Near", nearClipPlane)) {
			target_->SetNearClipPlane(nearClipPlane);
		}

		float farClipPlane = target_->GetFarClipPlane();
		if (GUI::FloatField("Far", farClipPlane)) {
			target_->SetFarClipPlane(farClipPlane);
		}
	}
};
