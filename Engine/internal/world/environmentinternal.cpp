#include "environmentinternal.h"

void EnvironmentInternal::SetSkybox(Material value) {
	skybox_ = value;
	skybox_->SetRenderQueue((int)RenderQueue::Geometry + 1000);
}
