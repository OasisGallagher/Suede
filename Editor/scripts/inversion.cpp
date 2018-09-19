#include "inversion.h"

#include "graphics.h"
#include "resources.h"

SUEDE_DEFINE_COMPONENT(Inversion, ImageEffect)

void Inversion::Awake() {
	material_ = NewMaterial();
	material_->SetShader(Resources::instance()->FindShader("inversion"));
}

void Inversion::OnRenderImage(RenderTexture src, RenderTexture dest, const Rect& normalizedRect) {
	Graphics::instance()->Blit(src, dest, material_, normalizedRect);
}
