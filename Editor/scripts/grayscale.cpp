#include "grayscale.h"

#include "engine.h"
#include "graphics.h"
#include "resources.h"

SUEDE_DEFINE_COMPONENT(Grayscale, ImageEffect)

void Grayscale::Awake() {
	graphics_ = Engine::GetSubsystem<Graphics>();

	material_ = new Material();
	material_->SetShader(Shader::Find("grayscale"));
}

void Grayscale::OnRenderImage(RenderTexture* src, RenderTexture* dest, const Rect& normalizedRect) {
	graphics_->Blit(src, dest, material_.get(), normalizedRect);
}
