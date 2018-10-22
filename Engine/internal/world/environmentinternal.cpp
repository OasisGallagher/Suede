#include "environmentinternal.h"

#include "memory/memory.h"

Environment::Environment() : Singleton2<Environment>(MEMORY_NEW(EnvironmentInternal), Memory::DeleteRaw<EnvironmentInternal>) {}
void Environment::SetSkybox(Material value) { _suede_dptr()->SetSkybox(value); }
Material Environment::GetSkybox() { return _suede_dptr()->GetSkybox(); }
void Environment::SetAmbientColor(const Color& value) { _suede_dptr()->SetAmbientColor(value); }
Color Environment::GetAmbientColor() { return _suede_dptr()->GetAmbientColor(); }
void Environment::SetFogColor(const Color& value) { _suede_dptr()->SetFogColor(value); }
Color Environment::GetFogColor() { return _suede_dptr()->GetFogColor(); }
void Environment::SetFogDensity(float value) { _suede_dptr()->SetFogDensity(value); }
float Environment::GetFogDensity() { return _suede_dptr()->GetFogDensity(); }

void EnvironmentInternal::SetSkybox(Material value) {
	skybox_ = value;
	skybox_->SetRenderQueue((int)RenderQueue::Geometry + 1000);
}
