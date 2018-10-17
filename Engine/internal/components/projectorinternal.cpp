#include "frustum.h"
#include "projectorinternal.h"

IProjector::IProjector() : IComponent(MEMORY_NEW(ProjectorInternal)) {}
bool IProjector::GetPerspective() const { return _dptr()->GetPerspective(); }
void IProjector::SetPerspective(bool value) { _dptr()->SetPerspective(value); }
float IProjector::GetOrthographicSize() const { return _dptr()->GetOrthographicSize(); }
void IProjector::SetOrthographicSize(float value) { _dptr()->SetOrthographicSize(value); }
Texture IProjector::GetTexture() const { return _dptr()->GetTexture(); }
void IProjector::SetTexture(Texture value) { _dptr()->SetTexture(value); }
void IProjector::SetDepth(int value) { _dptr()->SetDepth(value); }
int IProjector::GetDepth() const { return _dptr()->GetDepth(); }
void IProjector::SetAspect(float value) { _dptr()->SetAspect(value); }
void IProjector::SetNearClipPlane(float value) { _dptr()->SetNearClipPlane(value); }
void IProjector::SetFarClipPlane(float value) { _dptr()->SetFarClipPlane(value); }
void IProjector::SetFieldOfView(float value) { _dptr()->SetFieldOfView(value); }
float IProjector::GetAspect() const { return _dptr()->GetAspect(); }
float IProjector::GetNearClipPlane() const { return _dptr()->GetNearClipPlane(); }
float IProjector::GetFarClipPlane() const { return _dptr()->GetFarClipPlane(); }
float IProjector::GetFieldOfView() const { return _dptr()->GetFieldOfView(); }
const glm::mat4& IProjector::GetProjectionMatrix() { return _dptr()->GetProjectionMatrix(); }

SUEDE_DEFINE_COMPONENT(IProjector, IComponent)

ProjectorInternal::ProjectorInternal()
	: ComponentInternal(ObjectType::Projector), depth_(0) {
}
