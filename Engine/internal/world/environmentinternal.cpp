#include "environmentinternal.h"

void EnvironmentInternal::SetSkybox(Material value) {
	skybox_ = value;
	skybox_->SetRenderQueue(RenderQueueGeometry - 1000);
}
