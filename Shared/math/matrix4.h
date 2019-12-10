#pragma once
#include "../types.h"

#include "vector3.h"
#include "vector4.h"
#include "quaternion.h"

struct SUEDE_API Matrix4 {
	Vector4 m[4];

	Matrix4() { m[0].x = m[1].y = m[2].z = m[3].w = 1; }
	explicit Matrix4(float scalar) { m[0].x = m[1].y = m[2].z = m[3].w = scalar; }
	Matrix4(const Vector4& c0, const Vector4& c1, const Vector4& c2, const Vector4& c3) { m[0] = c0; m[1] = c1; m[2] = c2; m[3] = c3; }
	Matrix4(float x0, float y0, float z0, float w0, float x1, float y1, float z1, float w1, float x2, float y2, float z2, float w2, float x3, float y3, float z3, float w3);

	Matrix4 GetInversed() const;
	Matrix4 GetTransposed() const;
	float GetDeterminant() const;

	Vector4& operator[](int i) { return m[i]; }
	const Vector4& operator[](int i) const { return m[i]; }

	Vector4 operator*(const Vector4& v) const;

	Matrix4 operator*(const Matrix4& other) const;
	Matrix4& operator*=(const Matrix4& other);

	static Matrix4 Translate(const Vector3& t);

	static Matrix4 Rotate(const Quaternion& q);

	static Matrix4 Scale(const Vector3& s);

	static Matrix4 Ortho(float left, float right, float bottom, float top, float near, float far);

	static Vector3 Project(Vector3 const& obj, Matrix4 const& model, Matrix4 const& proj, Vector4 const& viewport);
	static Vector3 Unproject(Vector3 const& win, Matrix4 const& model, Matrix4 const& proj, Vector4 const& viewport);

	static Matrix4 LookAt(Vector3 const & eye, Vector3 const & center, Vector3 const & up);

	static void Inverse(Matrix4& matrix) { matrix = matrix.GetInversed(); }
	static void Transpose(Matrix4& matrix) { matrix = matrix.GetTransposed(); }

	static Matrix4 Perspective(float fov, float aspect, float near, float far);
	static Matrix4 TRS(const Vector3& t, const Quaternion& r, const Vector3& s) { return Translate(t) * Rotate(r) * Scale(s); }
	static bool Decompose(const Matrix4& matrix, Vector3& scale, Quaternion& orientation, Vector3& translation, Vector3& skew, Vector4& perspective);
};
