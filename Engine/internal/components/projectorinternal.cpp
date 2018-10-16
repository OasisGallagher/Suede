#include "frustum.h"
#include "projectorinternal.h"

IProjector::IProjector() : IComponent(MEMORY_NEW(ProjectorInternal)) {}
bool IProjector::GetPerspective() const { return dptr()->GetPerspective(); }
void IProjector::SetPerspective(bool value) { dptr()->SetPerspective(value); }
float IProjector::GetOrthographicSize() const { return dptr()->GetOrthographicSize(); }
void IProjector::SetOrthographicSize(float value) { dptr()->SetOrthographicSize(value); }
Texture IProjector::GetTexture() const { return dptr()->GetTexture(); }
void IProjector::SetTexture(Texture value) { dptr()->SetTexture(value); }
void IProjector::SetDepth(int value) { dptr()->SetDepth(value); }
int IProjector::GetDepth() const { return dptr()->GetDepth(); }
void IProjector::SetAspect(float value) { dptr()->SetAspect(value); }
void IProjector::SetNearClipPlane(float value) { dptr()->SetNearClipPlane(value); }
void IProjector::SetFarClipPlane(float value) { dptr()->SetFarClipPlane(value); }
void IProjector::SetFieldOfView(float value) { dptr()->SetFieldOfView(value); }
float IProjector::GetAspect() const { return dptr()->GetAspect(); }
float IProjector::GetNearClipPlane() const { return dptr()->GetNearClipPlane(); }
float IProjector::GetFarClipPlane() const { return dptr()->GetFarClipPlane(); }
float IProjector::GetFieldOfView() const { return dptr()->GetFieldOfView(); }
const glm::mat4& IProjector::GetProjectionMatrix() { return dptr()->GetProjectionMatrix(); }

SUEDE_DEFINE_COMPONENT(IProjector, IComponent)

ProjectorInternal::ProjectorInternal()
	: ComponentInternal(ObjectType::Projector), depth_(0) {
}
