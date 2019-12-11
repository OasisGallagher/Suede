#include "environmentinternal.h"

#include "memory/memory.h"

Environment::Environment() : Singleton2<Environment>(MEMORY_NEW(EnvironmentInternal), Memory::DeleteRaw<EnvironmentInternal>) {}
void Environment::SetSkybox(Material* value) { _suede_dinstance()->SetSkybox(value); }
Material* Environment::GetSkybox() { return _suede_dinstance()->GetSkybox(); }
void Environment::SetAmbientColor(const Color& value) { _suede_dinstance()->SetAmbientColor(value); }
Color Environment::GetAmbientColor() { return _suede_dinstance()->GetAmbientColor(); }
void Environment::SetFogColor(const Color& value) { _suede_dinstance()->SetFogColor(value); }
Color Environment::GetFogColor() { return _suede_dinstance()->GetFogColor(); }
void Environment::SetFogDensity(float value) { _suede_dinstance()->SetFogDensity(value); }
float Environment::GetFogDensity() { return _suede_dinstance()->GetFogDensity(); }

void EnvironmentInternal::SetSkybox(Material* value) {
	skybox_ = value;
	skybox_->SetRenderQueue((int)RenderQueue::Geometry + 1000);
}
