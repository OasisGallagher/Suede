#include "inversion.h"

#include "engine.h"
#include "graphics.h"

SUEDE_DEFINE_COMPONENT(Inversion, ImageEffect)

void Inversion::Awake() {
	graphics_ = Engine::GetSubsystem<Graphics>();
	material_ = new Material();
	material_->SetShader(Shader::Find("inversion"));
}

void Inversion::OnRenderImage(RenderTexture* src, RenderTexture* dest, const Rect& normalizedRect) {
	graphics_->Blit(src, dest, material_.get(), normalizedRect);
}
