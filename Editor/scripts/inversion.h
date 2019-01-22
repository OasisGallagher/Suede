#pragma once
#include <QObject>
#include "material.h"
#include "imageeffect.h"

class Inversion : public QObject, public ImageEffect {
	Q_OBJECT
	SUEDE_DECLARE_COMPONENT()

public:
	virtual void Awake();
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest, const Rect& normalizedRect);

private:
	Material material_;
};
