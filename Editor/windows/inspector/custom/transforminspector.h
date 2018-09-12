#include "gui.h"
#include "rect.h"
#include "transform.h"
#include "custominspector.h"

class TransformInspector : public CustomInspectorT<Transform> {
public:
	virtual void onGui() {
		glm::vec3 v3 = target_->GetLocalPosition();
		if (GUI::Float3Field("P", v3)) {
			target_->SetPosition(v3);
		}

		v3 = target_->GetLocalEulerAngles();
		if (GUI::Float3Field("R", v3)) {
			target_->SetLocalEulerAngles(v3);
		}

		v3 = target_->GetLocalScale();
		if (GUI::Float3Field("S", v3)) {
			target_->SetLocalScale(v3);
		}
	}
};
