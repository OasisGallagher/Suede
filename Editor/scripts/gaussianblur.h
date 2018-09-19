#pragma once
#include <QObject>

#include "ranged.h"
#include "graphics.h"
#include "imageeffect.h"

#include "../windows/inspector/custom/componentmetaobject.h"

class GaussianBlur : public QObject, public ImageEffect {
	Q_OBJECT
	Q_PROPERTY(RangedUInt Amount READ amount WRITE setAmount__)
	SUEDE_DECLARE_COMPONENT()

public:
	virtual void Awake();
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest, const Rect& normalizedRect);

public:
	void setAmount(uint value) { amount_ = value; }

	void setAmount__(RangedUInt value) { amount_ = value; }
	RangedUInt amount() const { return amount_; }

private:
	Material material_;
	RangedUInt amount_;
};
