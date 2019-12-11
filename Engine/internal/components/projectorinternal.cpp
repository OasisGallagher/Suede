#include "frustum.h"
#include "projectorinternal.h"

Projector::Projector() : Component(MEMORY_NEW(ProjectorInternal)) {}
bool Projector::GetPerspective() const { return _suede_dptr()->GetPerspective(); }
void Projector::SetPerspective(bool value) { _suede_dptr()->SetPerspective(value); }
float Projector::GetOrthographicSize() const { return _suede_dptr()->GetOrthographicSize(); }
void Projector::SetOrthographicSize(float value) { _suede_dptr()->SetOrthographicSize(value); }
Texture* Projector::GetTexture() const { return _suede_dptr()->GetTexture(); }
void Projector::SetTexture(Texture* value) { _suede_dptr()->SetTexture(value); }
void Projector::SetDepth(int value) { _suede_dptr()->SetDepth(value); }
int Projector::GetDepth() const { return _suede_dptr()->GetDepth(); }
void Projector::SetAspect(float value) { _suede_dptr()->SetAspect(value); }
void Projector::SetNearClipPlane(float value) { _suede_dptr()->SetNearClipPlane(value); }
void Projector::SetFarClipPlane(float value) { _suede_dptr()->SetFarClipPlane(value); }
void Projector::SetFieldOfView(float value) { _suede_dptr()->SetFieldOfView(value); }
float Projector::GetAspect() const { return _suede_dptr()->GetAspect(); }
float Projector::GetNearClipPlane() const { return _suede_dptr()->GetNearClipPlane(); }
float Projector::GetFarClipPlane() const { return _suede_dptr()->GetFarClipPlane(); }
float Projector::GetFieldOfView() const { return _suede_dptr()->GetFieldOfView(); }
const Matrix4& Projector::GetProjectionMatrix() { return _suede_dptr()->GetProjectionMatrix(); }

SUEDE_DEFINE_COMPONENT_INTERNAL(Projector, Component)

ProjectorInternal::ProjectorInternal()
	: ComponentInternal(ObjectType::Projector), depth_(0) {
}
