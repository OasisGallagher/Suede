#pragma once
#include "math/matrix4.h"

class Frustum {
public:
	Frustum();

public:
	bool GetPerspective() const { return perspective_; }
	void SetPerspective(bool value);

	float GetOrthographicSize() const;
	void SetOrthographicSize(float value);
	void SetAspect(float value);
	float GetAspect() const { return aspect_; }

	void SetNearClipPlane(float value);
	float GetNearClipPlane() const { return near_; }

	void SetFarClipPlane(float value);
	float GetFarClipPlane() const { return far_; }

	void SetFieldOfView(float value);
	float GetFieldOfView() const;

	const Matrix4& GetProjectionMatrix();

protected:
	virtual void OnProjectionMatrixChanged() {}

private:
	void DirtyProjectionMatrix();
	void CalculateProjectionMatrix();

private:
	float aspect_;
	float near_, far_;
	bool perspective_;
	float fieldOfView_;

	float orthographicSize_;

	bool matrixDirty_;
	Matrix4 projection_;
};
