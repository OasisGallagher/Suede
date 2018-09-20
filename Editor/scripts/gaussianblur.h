#pragma once
#include <QObject>

#include "ranged.h"
#include "graphics.h"
#include "imageeffect.h"

#include "../windows/inspector/custom/componentmetaobject.h"

class GaussianBlur : public QObject, public ImageEffect {
	Q_OBJECT
	Q_PROPERTY(Material Material READ material)
	Q_PROPERTY(iranged Amount READ amount WRITE setAmount)
	SUEDE_DECLARE_COMPONENT()

public:
	virtual void Awake();
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest, const Rect& normalizedRect);

public:
	Material material() { return material_; }

	void setAmount(const iranged& value) { amount_ = value; }
	iranged amount() const { return amount_; }

private:
	Material material_;
	iranged amount_;
};
