#include "matrix4.h"

#include <cmath>
#include <cstring>

Matrix4::Matrix4(float x0, float y0, float z0, float w0, float x1, float y1, float z1, float w1, float x2, float y2, float z2, float w2, float x3, float y3, float z3, float w3) {
	m[0] = Vector4(x0, y0, z0, w0);
	m[1] = Vector4(x1, y1, z1, w1);
	m[2] = Vector4(x2, y2, z2, w2);
	m[3] = Vector4(x3, y3, z3, w3);
}

Matrix4 Matrix4::GetInversed() const {
	float Coef00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
	float Coef02 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
	float Coef03 = m[1][2] * m[2][3] - m[2][2] * m[1][3];

	float Coef04 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
	float Coef06 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
	float Coef07 = m[1][1] * m[2][3] - m[2][1] * m[1][3];

	float Coef08 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
	float Coef10 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
	float Coef11 = m[1][1] * m[2][2] - m[2][1] * m[1][2];

	float Coef12 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
	float Coef14 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
	float Coef15 = m[1][0] * m[2][3] - m[2][0] * m[1][3];

	float Coef16 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
	float Coef18 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
	float Coef19 = m[1][0] * m[2][2] - m[2][0] * m[1][2];

	float Coef20 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
	float Coef22 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
	float Coef23 = m[1][0] * m[2][1] - m[2][0] * m[1][1];

	Vector4 Fac0(Coef00, Coef00, Coef02, Coef03);
	Vector4 Fac1(Coef04, Coef04, Coef06, Coef07);
	Vector4 Fac2(Coef08, Coef08, Coef10, Coef11);
	Vector4 Fac3(Coef12, Coef12, Coef14, Coef15);
	Vector4 Fac4(Coef16, Coef16, Coef18, Coef19);
	Vector4 Fac5(Coef20, Coef20, Coef22, Coef23);

	Vector4 Vec0(m[1][0], m[0][0], m[0][0], m[0][0]);
	Vector4 Vec1(m[1][1], m[0][1], m[0][1], m[0][1]);
	Vector4 Vec2(m[1][2], m[0][2], m[0][2], m[0][2]);
	Vector4 Vec3(m[1][3], m[0][3], m[0][3], m[0][3]);

	Vector4 Inv0(Vec1 * Fac0 - Vec2 * Fac1 + Vec3 * Fac2);
	Vector4 Inv1(Vec0 * Fac0 - Vec2 * Fac3 + Vec3 * Fac4);
	Vector4 Inv2(Vec0 * Fac1 - Vec1 * Fac3 + Vec3 * Fac5);
	Vector4 Inv3(Vec0 * Fac2 - Vec1 * Fac4 + Vec2 * Fac5);

	Vector4 SignA(+1, -1, +1, -1);
	Vector4 SignB(-1, +1, -1, +1);
	Matrix4 Inverse(Inv0 * SignA, Inv1 * SignB, Inv2 * SignA, Inv3 * SignB);

	Vector4 Row0(Inverse[0][0], Inverse[1][0], Inverse[2][0], Inverse[3][0]);

	Vector4 Dot0(m[0] * Row0);
	float Dot1 = (Dot0.x + Dot0.y) + (Dot0.z + Dot0.w);

	float OneOverDeterminant = static_cast<float>(1) / Dot1;

	Inverse.m[0] *= OneOverDeterminant;
	Inverse.m[1] *= OneOverDeterminant;
	Inverse.m[2] *= OneOverDeterminant;
	Inverse.m[3] *= OneOverDeterminant;

	return Inverse;
}

Matrix4 Matrix4::LookAt(Vector3 const & eye, Vector3 const & center, Vector3 const & up) {
	Vector3 const f((center - eye).GetNormalized());
	Vector3 const s(Vector3::Cross(f, up).GetNormalized());
	Vector3 const u(Vector3::Cross(s, f));

	Matrix4 Result(1);
	Result[0][0] = s.x;
	Result[1][0] = s.y;
	Result[2][0] = s.z;
	Result[0][1] = u.x;
	Result[1][1] = u.y;
	Result[2][1] = u.z;
	Result[0][2] = -f.x;
	Result[1][2] = -f.y;
	Result[2][2] = -f.z;
	Result[3][0] = -Vector3::Dot(s, eye);
	Result[3][1] = -Vector3::Dot(u, eye);
	Result[3][2] = Vector3::Dot(f, eye);
	return Result;
}

Matrix4 Matrix4::GetTransposed() const {
	return Matrix4(m[0][0], m[1][0], m[2][0], m[3][0],
		m[0][1], m[1][1], m[2][1], m[3][1],
		m[0][2], m[1][2], m[2][2], m[3][2],
		m[0][3], m[1][3], m[2][3], m[3][3]
	);
}

float Matrix4::GetDeterminant() const {
	float SubFactor00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
	float SubFactor01 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
	float SubFactor02 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
	float SubFactor03 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
	float SubFactor04 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
	float SubFactor05 = m[2][0] * m[3][1] - m[3][0] * m[2][1];

	Vector4 DetCof(
		+(m[1][1] * SubFactor00 - m[1][2] * SubFactor01 + m[1][3] * SubFactor02),
		-(m[1][0] * SubFactor00 - m[1][2] * SubFactor03 + m[1][3] * SubFactor04),
		+(m[1][0] * SubFactor01 - m[1][1] * SubFactor03 + m[1][3] * SubFactor05),
		-(m[1][0] * SubFactor02 - m[1][1] * SubFactor04 + m[1][2] * SubFactor05)
	);

	return m[0][0] * DetCof[0] + m[0][1] * DetCof[1] + m[0][2] * DetCof[2] + m[0][3] * DetCof[3];
}

bool Matrix4::Decompose(const Matrix4& matrix, Vector3& scale, Quaternion& orientation, Vector3& translation, Vector3& skew, Vector4& perspective) {
	Matrix4 LocalMatrix(matrix);

	// Normalize the matrix.
	if (LocalMatrix[3][3] == static_cast<float>(0))
		return false;

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			LocalMatrix[i][j] /= LocalMatrix[3][3];
		}
	}

	// perspectiveMatrix is used to solve for perspective, but it also provides
	// an easy way to test for singularity of the upper 3x3 component.
	Matrix4 PerspectiveMatrix(LocalMatrix);

	for (int i = 0; i < 3; i++) {
		PerspectiveMatrix[i][3] = static_cast<float>(0);
	}

	PerspectiveMatrix[3][3] = static_cast<float>(1);

	/// TODO: Fixme!
	if (PerspectiveMatrix.GetDeterminant() == static_cast<float>(0)) {
		return false;
	}

	// First, isolate perspective.  This is the messiest.
	if (LocalMatrix[0][3] != static_cast<float>(0) || LocalMatrix[1][3] != static_cast<float>(0) || LocalMatrix[2][3] != static_cast<float>(0)) {
		// rightHandSide is the right hand side of the equation.
		Vector4 RightHandSide;
		RightHandSide[0] = LocalMatrix[0][3];
		RightHandSide[1] = LocalMatrix[1][3];
		RightHandSide[2] = LocalMatrix[2][3];
		RightHandSide[3] = LocalMatrix[3][3];

		// Solve the equation by inverting PerspectiveMatrix and multiplying
		// rightHandSide by the inverse.  (This is the easiest way, not
		// necessarily the best.)
		Matrix4 InversePerspectiveMatrix = PerspectiveMatrix.GetInversed();//   inverse(PerspectiveMatrix, inversePerspectiveMatrix);
		Matrix4 TransposedInversePerspectiveMatrix = InversePerspectiveMatrix.GetTransposed();//   transposeMatrix4(inversePerspectiveMatrix, transposedInversePerspectiveMatrix);

		perspective = TransposedInversePerspectiveMatrix * RightHandSide;
		//  v4MulPointByMatrix(rightHandSide, transposedInversePerspectiveMatrix, perspectivePoint);

		// Clear the perspective partition
		LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<float>(0);
		LocalMatrix[3][3] = static_cast<float>(1);
	}
	else {
		// No perspective.
		perspective = Vector4(0, 0, 0, 1);
	}

	// Next take care of translation (easy).
	translation = Vector3(LocalMatrix[3].x, LocalMatrix[3].y, LocalMatrix[3].z);
	LocalMatrix[3] = Vector4(0, 0, 0, LocalMatrix[3].w);

	Vector3 Row[3], Pdum3;

	// Now get scale and shear.
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			Row[i][j] = LocalMatrix[i][j];
		}
	}

	// Compute X scale factor and normalize first row.
	scale.x = Row[0].GetMagnitude();// v3Length(Row[0]);

	Vector3::Normalize(Row[0]);

	// Compute XY shear factor and make 2nd row orthogonal to 1st.
	skew.z = Vector3::Dot(Row[0], Row[1]);
	Row[1] = Row[1] * 1.f + Row[0] * -skew.z;

	// Now, compute Y scale and normalize 2nd row.
	scale.y = Row[1].GetMagnitude();
	Vector3::Normalize(Row[1]);
	skew.z /= scale.y;

	// Compute XZ and YZ shears, orthogonalize 3rd row.
	skew.y = Vector3::Dot(Row[0], Row[2]);
	Row[2] = Row[2] * 1.f + Row[0] * -skew.y;
	skew.x = Vector3::Dot(Row[1], Row[2]);
	Row[2] = Row[2] * 1.f + Row[1] * -skew.x;

	// Next, get Z scale and normalize 3rd row.
	scale.z = Row[2].GetMagnitude();
	Vector3::Normalize(Row[2]);
	skew.y /= scale.z;
	skew.x /= scale.z;

	// At this point, the matrix (in rows[]) is orthonormal.
	// Check for a coordinate system flip.  If the determinant
	// is -1, then negate the matrix and the scaling factors.
	Pdum3 = Vector3::Cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
	if (Vector3::Dot(Row[0], Pdum3) < 0) {
		for (int i = 0; i < 3; i++) {
			scale.x *= static_cast<float>(-1);
			Row[i] *= static_cast<float>(-1);
		}
	}

	// Now, get the rotations out, as described in the gem.

	// FIXME - Add the ability to return either quaternions (which are
	// easier to recompose with) or Euler angles (rx, ry, rz), which
	// are easier for authors to deal with. The latter will only be useful
	// when we fix https://bugs.webkit.org/show_bug.cgi?id=23799, so I
	// will leave the Euler angle code here for now.

	// ret.rotateY = asin(-Row[0][2]);
	// if (cos(ret.rotateY) != 0) {
	//     ret.rotateX = atan2(Row[1][2], Row[2][2]);
	//     ret.rotateZ = atan2(Row[0][1], Row[0][0]);
	// } else {
	//     ret.rotateX = atan2(-Row[2][0], Row[1][1]);
	//     ret.rotateZ = 0;
	// }

	float s, t, x, y, z, w;

	t = Row[0][0] + Row[1][1] + Row[2][2] + static_cast<float>(1);

	if (t > static_cast<float>(1e-4)) {
		s = static_cast<float>(0.5) / sqrt(t);
		w = static_cast<float>(0.25) / s;
		x = (Row[2][1] - Row[1][2]) * s;
		y = (Row[0][2] - Row[2][0]) * s;
		z = (Row[1][0] - Row[0][1]) * s;
	}
	else if (Row[0][0] > Row[1][1] && Row[0][0] > Row[2][2]) {
		s = sqrt(static_cast<float>(1) + Row[0][0] - Row[1][1] - Row[2][2])* static_cast<float>(2); // S=4*qx 
		x = static_cast<float>(0.25)* s;
		y = (Row[0][1] + Row[1][0]) / s;
		z = (Row[0][2] + Row[2][0]) / s;
		w = (Row[2][1] - Row[1][2]) / s;
	}
	else if (Row[1][1] > Row[2][2]) {
		s = sqrt(static_cast<float>(1) + Row[1][1] - Row[0][0] - Row[2][2])* static_cast<float>(2); // S=4*qy
		x = (Row[0][1] + Row[1][0]) / s;
		y = static_cast<float>(0.25)* s;
		z = (Row[1][2] + Row[2][1]) / s;
		w = (Row[0][2] - Row[2][0]) / s;
	}
	else {
		s = sqrt(static_cast<float>(1) + Row[2][2] - Row[0][0] - Row[1][1])* static_cast<float>(2); // S=4*qz
		x = (Row[0][2] + Row[2][0]) / s;
		y = (Row[1][2] + Row[2][1]) / s;
		z = static_cast<float>(0.25)* s;
		w = (Row[1][0] - Row[0][1]) / s;
	}

	orientation.x = x;
	orientation.y = y;
	orientation.z = z;
	orientation.w = w;

	return true;
}

Vector4 Matrix4::operator*(const Vector4& v) const {
	return Vector4(
		m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2] + m[3][0] * v[3],
		m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2] + m[3][1] * v[3],
		m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2] + m[3][2] * v[3],
		m[0][3] * v[0] + m[1][3] * v[1] + m[2][3] * v[2] + m[3][3] * v[3]
	);
}

Matrix4 Matrix4::operator*(const Matrix4& other) const {
	return Matrix4(*this) *= other;
}

Matrix4& Matrix4::operator*=(const Matrix4& other) {
	Vector4 SrcA0 = m[0];
	Vector4 SrcA1 = m[1];
	Vector4 SrcA2 = m[2];
	Vector4 SrcA3 = m[3];

	Vector4 SrcB0 = other.m[0];
	Vector4 SrcB1 = other.m[1];
	Vector4 SrcB2 = other.m[2];
	Vector4 SrcB3 = other.m[3];

	m[0] = SrcA0 * SrcB0[0] + SrcA1 * SrcB0[1] + SrcA2 * SrcB0[2] + SrcA3 * SrcB0[3];
	m[1] = SrcA0 * SrcB1[0] + SrcA1 * SrcB1[1] + SrcA2 * SrcB1[2] + SrcA3 * SrcB1[3];
	m[2] = SrcA0 * SrcB2[0] + SrcA1 * SrcB2[1] + SrcA2 * SrcB2[2] + SrcA3 * SrcB2[3];
	m[3] = SrcA0 * SrcB3[0] + SrcA1 * SrcB3[1] + SrcA2 * SrcB3[2] + SrcA3 * SrcB3[3];

	return *this;
}

/**
 * Given that most matrices will be transformation matrices, the elements
 * are tested in order such that the test is likely to fail as early as possible.
 */
bool Matrix4::operator==(const Matrix4& other) const {
	return m[3] == other.m[3] && m[2] == other.m[2] && m[1] == other.m[1] && m[0] == other.m[0];
}

bool Matrix4::operator!=(const Matrix4& other) const {
	return m[3] != other.m[3] || m[2] != other.m[2] || m[1] != other.m[1] || m[0] != other.m[0];
}

Matrix4 Matrix4::Translate(const Vector3& t) {
	Matrix4 Result(1);
	Result[3] = Vector4(t.x, t.y, t.z, 1);
	return Result;
}

Matrix4 Matrix4::Rotate(const Quaternion& q) {
	Matrix4 Result(1);
	float qxx(q.x * q.x);
	float qyy(q.y * q.y);
	float qzz(q.z * q.z);
	float qxz(q.x * q.z);
	float qxy(q.x * q.y);
	float qyz(q.y * q.z);
	float qwx(q.w * q.x);
	float qwy(q.w * q.y);
	float qwz(q.w * q.z);

	Result[0][0] = 1 - 2 * (qyy + qzz);
	Result[0][1] = 2 * (qxy + qwz);
	Result[0][2] = 2 * (qxz - qwy);

	Result[1][0] = 2 * (qxy - qwz);
	Result[1][1] = 1 - 2 * (qxx + qzz);
	Result[1][2] = 2 * (qyz + qwx);

	Result[2][0] = 2 * (qxz + qwy);
	Result[2][1] = 2 * (qyz - qwx);
	Result[2][2] = 1 - 2 * (qxx + qyy);

	return Result;
}

Matrix4 Matrix4::Scale(const Vector3& s) {
	Matrix4 Result;
	Result[0][0] = s[0];
	Result[1][1] = s[1];
	Result[2][2] = s[2];
	Result[3][3] = 1;
	return Result;
}

Matrix4 Matrix4::Ortho(float left, float right, float bottom, float top, float near, float far) {
	Matrix4 Result(1);
	Result[0][0] = 2 / (right - left);
	Result[1][1] = 2 / (top - bottom);
	Result[2][2] = -2 / (far - near);
	Result[3][0] = -(right + left) / (right - left);
	Result[3][1] = -(top + bottom) / (top - bottom);
	Result[3][2] = -(far + near) / (far - near);

	return Result;
}

Vector3 Matrix4::Project(Vector3 const& obj, Matrix4 const& model, Matrix4 const& proj, IVector4 const& viewport) {
	Vector4 tmp = Vector4(obj.x, obj.y, obj.z, 1);
	tmp = model * tmp;
	tmp = proj * tmp;

	tmp /= tmp.w;
	tmp = tmp * (0.5f) + Vector4(0.5f);
	tmp[0] = tmp[0] * (viewport[2]) + (viewport[0]);
	tmp[1] = tmp[1] * (viewport[3]) + (viewport[1]);

	return Vector3(tmp.x, tmp.y, tmp.z);
}

Vector3 Matrix4::Unproject(Vector3 const& win, Matrix4 const& model, Matrix4 const& proj, IVector4 const& viewport) {
	Matrix4 Inverse = (proj * model).GetInversed();

	Vector4 tmp = Vector4(win.x, win.y, win.z, (1));
	tmp.x = (tmp.x - (viewport[0])) / (viewport[2]);
	tmp.y = (tmp.y - (viewport[1])) / (viewport[3]);
	tmp = tmp * (2) - Vector4(1);

	Vector4 obj = Inverse * tmp;
	obj /= obj.w;

	return Vector3(obj.x, obj.y, obj.z);
}

Matrix4 Matrix4::Perspective(float fovy, float aspect, float near, float far) {
	float tanHalfFovy = tanf(fovy / 2);

	Matrix4 Result(0);
	Result[0][0] = (1) / (aspect * tanHalfFovy);
	Result[1][1] = (1) / (tanHalfFovy);
	Result[2][2] = -(far + near) / (far - near);
	Result[2][3] = -(1);
	Result[3][2] = -((2) * far * near) / (far - near);

	return Result;
}
