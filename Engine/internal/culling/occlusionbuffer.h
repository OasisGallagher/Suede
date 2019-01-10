#pragma once

#include <vector>

#include "internal/physics/bullet/btBulletDynamicsCommon.h"

// https://github.com/erwincoumans/experiments/issues/9
// https://pybullet.org/Bullet/phpBB3/viewtopic.php?f=9&t=7575
// This is actually a small "Sofware Rasterizer", that draws to and performs queries on the occlusion buffer
struct OcclusionBuffer {
	// 95% of the code is copied from CDTestFramework, but in this demo only a few of the appendOccluder/queryOccluder overloads are used
	struct WriteOCL {
		static bool Process(float& q, float v) { if (q < v) q = v; return(false); }
	};

	struct QueryOCL {
		static bool Process(float& q, float v) { return(q <= v); }
	};

	OcclusionBuffer() : neardist(2, 2, 2), ocarea(0), qrarea(0) {
		//texture = 0;
	}

	void setup(int w, int h);
	void clear();
	void reset() {}

	void initialize(const float* modelViewProjMatrix, int bufferWidth = 128, int bufferHeight = 128);
	void getBufferTextureData(std::vector<unsigned char>& data);

	btVector4 transform(const btVector3& x) const;

	static bool	project(btVector4* p, int n);

	template <const int NP>
	static int clip(const btVector4* pi, btVector4* po);

	template <class POLICY>
	bool draw(const btVector4& a, const btVector4& b, const btVector4& c, const float minarea);

	template <const int NP, class POLICY>
	bool clipDraw(const btVector4* p, float minarea);

	void appendOccluder(const btVector3& a, const btVector3& b, const btVector3& c);
	void appendOccluder(const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& d);
	void appendOccluder(const btVector3& c, const btVector3& e);	// in this class 'e' seems to actually be a half extent...
	void appendOccluder(const btVector3& occluderInnerBoxCollisionShapeHalfExtent, const btTransform& collisionObjectWorldTransform);

	bool queryOccluder(const btVector3& c, const btVector3& e);
	bool queryOccluder(const btVector3& a, const btVector3& b, const btVector3& c);
	bool queryOccluder(const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& d);

	int sizes[2];

	float wtrs[16];
	float scales[2];
	float offsets[2];

	btVector3 eye;
	btVector3 neardist;
	float ocarea;
	float qrarea;

	btAlignedObjectArray<float>	buffer;
};

inline void OcclusionBuffer::clear() {
	buffer.resize(0);
	buffer.resize(sizes[0] * sizes[1], 0);
}

template <const int NP>
int OcclusionBuffer::clip(const btVector4* pi, btVector4* po) {
	float	s[NP];
	int			m = 0;
	for (int i = 0; i < NP; ++i) {
		s[i] = pi[i][2] + pi[i][3];
		if (s[i] < 0) m += 1 << i;
	}
	if (m == ((1 << NP) - 1)) return(0);
	if (m != 0) {
		int n = 0;
		for (int i = NP - 1, j = 0; j < NP; i = j++) {
			const btVector4&	a = pi[i];
			const btVector4&	b = pi[j];
			const float		t = s[i] / (a[3] + a[2] - b[3] - b[2]);
			if ((t > 0) && (t < 1)) {
				po[n][0] = a[0] + (b[0] - a[0])*t;
				po[n][1] = a[1] + (b[1] - a[1])*t;
				po[n][2] = a[2] + (b[2] - a[2])*t;
				po[n][3] = a[3] + (b[3] - a[3])*t;
				++n;
			}
			if (s[j] > 0) po[n++] = b;
		}
		return(n);
	}

	for (int i = 0; i < NP; ++i) po[i] = pi[i];

	return(NP);
}

template <const int NP, class POLICY>
bool OcclusionBuffer::clipDraw(const btVector4* p, float minarea) {
	btVector4	o[NP * 2];
	const int	n = clip<NP>(p, o);
	bool		earlyexit = false;
	project(o, n);
	for (int i = 2; i < n; ++i) {
		earlyexit |= draw<POLICY>(o[0], o[i - 1], o[i], minarea);
	}

	return(earlyexit);
}

template <class POLICY>
bool OcclusionBuffer::draw(const btVector4& a, const btVector4& b, const btVector4& c, const float minarea) {
	const float		a2 = ((b - a).cross(c - a))[2];
	if (a2 > 0) {
		if (a2 < minarea)	return(true);
		const int x[] = {
			(int)(a.x()*scales[0] + offsets[0]),
			(int)(b.x()*scales[0] + offsets[0]),
			(int)(c.x()*scales[0] + offsets[0])
		};

		const int y[] = {
			(int)(a.y()*scales[1] + offsets[1]),
			(int)(b.y()*scales[1] + offsets[1]),
			(int)(c.y()*scales[1] + offsets[1])
		};

		const float	z[] = { a.z(),b.z(),c.z() };
		const int		mix = btMax(0, btMin(x[0], btMin(x[1], x[2])));
		const int		mxx = btMin(sizes[0], 1 + btMax(x[0], btMax(x[1], x[2])));
		const int		miy = btMax(0, btMin(y[0], btMin(y[1], y[2])));
		const int		mxy = btMin(sizes[1], 1 + btMax(y[0], btMax(y[1], y[2])));
		const int		width = mxx - mix;
		const int		height = mxy - miy;
		if ((width*height) > 0) {
			const int dx[] = {
				y[0] - y[1],
				y[1] - y[2],
				y[2] - y[0]
			};

			const int dy[] = {
				x[1] - x[0] - dx[0] * width,
				x[2] - x[1] - dx[1] * width,
				x[0] - x[2] - dx[2] * width
			};

			const int a = x[2] * y[0] + x[0] * y[1] - x[2] * y[1] - x[0] * y[2] + x[1] * y[2] - x[1] * y[0];
			const float	ia = 1 / (float)a;
			const float	dzx = ia*(y[2] * (z[1] - z[0]) + y[1] * (z[0] - z[2]) + y[0] * (z[2] - z[1]));
			const float	dzy = ia*(x[2] * (z[0] - z[1]) + x[0] * (z[1] - z[2]) + x[1] * (z[2] - z[0])) - (dzx*width);
			int	c[] = {
				miy*x[1] + mix*y[0] - x[1] * y[0] - mix*y[1] + x[0] * y[1] - miy*x[0],
				miy*x[2] + mix*y[1] - x[2] * y[1] - mix*y[2] + x[1] * y[2] - miy*x[1],
				miy*x[0] + mix*y[2] - x[0] * y[2] - mix*y[0] + x[2] * y[0] - miy*x[2]
			};

			float v = ia*((z[2] * c[0]) + (z[0] * c[1]) + (z[1] * c[2]));
			float* scan = &buffer[miy*sizes[1]];
			for (int iy = miy; iy < mxy; ++iy) {
				for (int ix = mix; ix < mxx; ++ix) {
					if ((c[0] >= 0) && (c[1] >= 0) && (c[2] >= 0)) {
						if (POLICY::Process(scan[ix], v)) return(true);
					}

					c[0] += dx[0]; c[1] += dx[1]; c[2] += dx[2]; v += dzx;
				}

				c[0] += dy[0]; c[1] += dy[1]; c[2] += dy[2]; v += dzy;
				scan += sizes[0];
			}
		}
	}

	return(false);
}

inline void OcclusionBuffer::appendOccluder(const btVector3& a, const btVector3& b, const btVector3& c) {
	const btVector4	p[] = { transform(a),transform(b),transform(c) };
	clipDraw<3, WriteOCL>(p, ocarea);
}

inline void OcclusionBuffer::appendOccluder(const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& d) {
	const btVector4	p[] = { transform(a),transform(b),transform(c),transform(d) };
	clipDraw<4, WriteOCL>(p, ocarea);
}

inline bool OcclusionBuffer::queryOccluder(const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& d) {
	const btVector4	p[] = { transform(a),transform(b),transform(c),transform(d) };
	return(clipDraw<4, QueryOCL>(p, qrarea));
}

inline bool OcclusionBuffer::queryOccluder(const btVector3& a, const btVector3& b, const btVector3& c) {
	const btVector4	p[] = { transform(a),transform(b),transform(c) };
	return(clipDraw<3, QueryOCL>(p, qrarea));
}
