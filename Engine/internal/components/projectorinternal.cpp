#include "frustum.h"
#include "projectorinternal.h"

IProjector::IProjector() : IComponent(MEMORY_NEW(ProjectorInternal, this)) {}
bool IProjector::GetPerspective() const { return _suede_dptr()->GetPerspective(); }
void IProjector::SetPerspective(bool value) { _suede_dptr()->SetPerspective(value); }
float IProjector::GetOrthographicSize() const { return _suede_dptr()->GetOrthographicSize(); }
void IProjector::SetOrthographicSize(float value) { _suede_dptr()->SetOrthographicSize(value); }
Texture IProjector::GetTexture() const { return _suede_dptr()->GetTexture(); }
void IProjector::SetTexture(Texture value) { _suede_dptr()->SetTexture(value); }
void IProjector::SetDepth(int value) { _suede_dptr()->SetDepth(value); }
int IProjector::GetDepth() const { return _suede_dptr()->GetDepth(); }
void IProjector::SetAspect(float value) { _suede_dptr()->SetAspect(value); }
void IProjector::SetNearClipPlane(float value) { _suede_dptr()->SetNearClipPlane(value); }
void IProjector::SetFarClipPlane(float value) { _suede_dptr()->SetFarClipPlane(value); }
void IProjector::SetFieldOfView(float value) { _suede_dptr()->SetFieldOfView(value); }
float IProjector::GetAspect() const { return _suede_dptr()->GetAspect(); }
float IProjector::GetNearClipPlane() const { return _suede_dptr()->GetNearClipPlane(); }
float IProjector::GetFarClipPlane() const { return _suede_dptr()->GetFarClipPlane(); }
float IProjector::GetFieldOfView() const { return _suede_dptr()->GetFieldOfView(); }
const glm::mat4& IProjector::GetProjectionMatrix() { return _suede_dptr()->GetProjectionMatrix(); }

SUEDE_DEFINE_COMPONENT_INTERNAL(Projector, Component)

ProjectorInternal::ProjectorInternal(IProjector* self)
	: ComponentInternal(self, ObjectType::Projector), depth_(0) {
}
