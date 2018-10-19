#include "environmentinternal.h"

#include "memory/memory.h"

#undef _dptr
#define _dptr()	((EnvironmentInternal*)d_)
Environment::Environment() : Singleton2<Environment>(MEMORY_NEW(EnvironmentInternal)) {}
void Environment::SetSkybox(Material value) { _dptr()->SetSkybox(value); }
Material Environment::GetSkybox() { return _dptr()->GetSkybox(); }
void Environment::SetAmbientColor(const Color& value) { _dptr()->SetAmbientColor(value); }
Color Environment::GetAmbientColor() { _dptr()->GetAmbientColor(); }
void Environment::SetFogColor(const Color& value) { _dptr()->SetFogColor(value); }
Color Environment::GetFogColor() { _dptr()->GetFogColor(); }
void Environment::SetFogDensity(float value) { _dptr()->SetFogDensity(value); }
float Environment::GetFogDensity() { _dptr()->GetFogDensity(); }

void EnvironmentInternal::SetSkybox(Material value) {
	skybox_ = value;
	skybox_->SetRenderQueue((int)RenderQueue::Geometry + 1000);
}
