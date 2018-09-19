#include "grayscale.h"

#include "graphics.h"
#include "resources.h"

SUEDE_DEFINE_COMPONENT(Grayscale, ImageEffect)

void Grayscale::Awake() {
	material_ = NewMaterial();
	material_->SetShader(Resources::instance()->FindShader("grayscale"));
}

void Grayscale::OnRenderImage(RenderTexture src, RenderTexture dest, const Rect& normalizedRect) {
	Graphics::instance()->Blit(src, dest, material_, normalizedRect);
}
