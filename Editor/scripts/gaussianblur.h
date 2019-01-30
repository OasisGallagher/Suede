#pragma once
#include <QObject>

#include "range.h"
#include "graphics.h"
#include "imageeffect.h"

#include "../windows/inspector/custom/componentmetaobject.h"

class GaussianBlur : public QObject, public ImageEffect {
	Q_OBJECT
	Q_PROPERTY(Material Material READ material)
	Q_PROPERTY(irange Amount READ amount WRITE setAmount)
	SUEDE_DECLARE_COMPONENT()

public:
	virtual void Awake();
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest, const Rect& normalizedRect);

public:
	Material material() { return material_; }

	void setAmount(const irange& value) { amount_ = value; }
	irange amount() const { return amount_; }

private:
	Material material_;
	irange amount_;
};
