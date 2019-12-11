#include "inversion.h"

#include "graphics.h"
#include "resources.h"

SUEDE_DEFINE_COMPONENT(Inversion, ImageEffect)

void Inversion::Awake() {
	material_ = new Material();
	material_->SetShader(Resources::FindShader("inversion"));
}

void Inversion::OnRenderImage(RenderTexture* src, RenderTexture* dest, const Rect& normalizedRect) {
	Graphics::Blit(src, dest, material_.get(), normalizedRect);
}
