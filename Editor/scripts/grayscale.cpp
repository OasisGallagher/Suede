#include "grayscale.h"

#include "graphics.h"
#include "resources.h"

SUEDE_DEFINE_COMPONENT(Grayscale, ImageEffect)

void Grayscale::Awake() {
	material_ = new Material();
	material_->SetShader(Resources::FindShader("grayscale"));
}

void Grayscale::OnRenderImage(RenderTexture* src, RenderTexture* dest, const Rect& normalizedRect) {
	Graphics::Blit(src, dest, material_.get(), normalizedRect);
}
