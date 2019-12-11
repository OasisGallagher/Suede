#pragma once
#include <QObject>
#include "material.h"
#include "imageeffect.h"

class Grayscale : public QObject, public ImageEffect {
	Q_OBJECT
	SUEDE_DECLARE_COMPONENT()

public:
	virtual void Awake();
	virtual void OnRenderImage(RenderTexture* src, RenderTexture* dest, const Rect& normalizedRect);

private:
	ref_ptr<Material> material_;
};
