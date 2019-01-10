#include "occlusionbuffer.h"

void OcclusionBuffer::setup(int w, int h) {
	reset();
	sizes[0] = w;
	sizes[1] = h;
	scales[0] = w * 0.5f;
	scales[1] = h * 0.5f;
	offsets[0] = scales[0] + 0.5f;
	offsets[1] = scales[1] + 0.5f;
	//glGenTextures(1, &texture);	// This was needed to display the occlusion buffer in CDTestFramework
	clear();
}

void OcclusionBuffer::initialize(const float* modelViewProjMatrix, int bufferWidth /*= 128*/, int bufferHeight /*= 128*/) {
	if (/*texture == 0 || */sizes[0] != bufferWidth || sizes[1] != bufferHeight) {
		setup(bufferWidth, bufferHeight);
	}

	for (int i = 0; i < 16; ++i) wtrs[i] = (float)modelViewProjMatrix[i];
	clear();
}

void OcclusionBuffer::getBufferTextureData(std::vector<unsigned char>& data) {
	data.resize(buffer.size());
	// Very slow, but good quality
	float bufferMax = data[0], bufferMin = data[0], bufferExtentFactor;
	for (int i = 0; i < buffer.size(); ++i) {
		const float d = buffer[i];
		if (bufferMin > d)	bufferMin = d;
		else if (bufferMax < d) bufferMax = d;
	}

	bufferExtentFactor = (bufferMax == bufferMin ? 0 : float(255.0) / (bufferMax - bufferMin));
	for (int i = 0; i < data.size(); ++i) {
		data[i] = (unsigned char)((buffer[i] - bufferMin)*bufferExtentFactor);
	}
}

btVector4 OcclusionBuffer::transform(const btVector3& x) const {
	btVector4	t;
	t[0] = x[0] * wtrs[0] + x[1] * wtrs[4] + x[2] * wtrs[8] + wtrs[12];
	t[1] = x[0] * wtrs[1] + x[1] * wtrs[5] + x[2] * wtrs[9] + wtrs[13];
	t[2] = x[0] * wtrs[2] + x[1] * wtrs[6] + x[2] * wtrs[10] + wtrs[14];
	t[3] = x[0] * wtrs[3] + x[1] * wtrs[7] + x[2] * wtrs[11] + wtrs[15];
	return(t);
}

bool OcclusionBuffer::project(btVector4* p, int n) {
	for (int i = 0; i < n; ++i) {
		const float	iw = 1 / p[i][3];
		p[i][2] = 1 / p[i][3];
		p[i][0] *= p[i][2];
		p[i][1] *= p[i][2];
	}

	return(true);
}

void OcclusionBuffer::appendOccluder(const btVector3& occluderInnerBoxCollisionShapeHalfExtent, const btTransform& collisionObjectWorldTransform) {
	const btVector3 c(collisionObjectWorldTransform.getOrigin());
	const btVector3& e = occluderInnerBoxCollisionShapeHalfExtent;
	const btMatrix3x3& basis = collisionObjectWorldTransform.getBasis();
	const btVector4	x[] = {
		transform(c + basis*btVector3(-e[0],-e[1],-e[2])),
		transform(c + basis*btVector3(+e[0],-e[1],-e[2])),
		transform(c + basis*btVector3(+e[0],+e[1],-e[2])),
		transform(c + basis*btVector3(-e[0],+e[1],-e[2])),
		transform(c + basis*btVector3(-e[0],-e[1],+e[2])),
		transform(c + basis*btVector3(+e[0],-e[1],+e[2])),
		transform(c + basis*btVector3(+e[0],+e[1],+e[2])),
		transform(c + basis*btVector3(-e[0],+e[1],+e[2]))
	};

	static const int	d[] = {
		1,0,3,2,
		4,5,6,7,
		4,7,3,0,
		6,5,1,2,
		7,6,2,3,
		5,4,0,1
	};

	for (int i = 0; i < (sizeof(d) / sizeof(d[0]));) {
		const btVector4	p[] = {
			x[d[i++]],
			x[d[i++]],
			x[d[i++]],
			x[d[i++]]
		};

		clipDraw<4, WriteOCL>(p, ocarea);
	}
}

void OcclusionBuffer::appendOccluder(const btVector3& c, const btVector3& e) // in this class 'e' seems to actually be a half extent...
{
	const btVector4	x[] = { transform(btVector3(c[0] - e[0],c[1] - e[1],c[2] - e[2])),
		transform(btVector3(c[0] + e[0],c[1] - e[1],c[2] - e[2])),
		transform(btVector3(c[0] + e[0],c[1] + e[1],c[2] - e[2])),
		transform(btVector3(c[0] - e[0],c[1] + e[1],c[2] - e[2])),
		transform(btVector3(c[0] - e[0],c[1] - e[1],c[2] + e[2])),
		transform(btVector3(c[0] + e[0],c[1] - e[1],c[2] + e[2])),
		transform(btVector3(c[0] + e[0],c[1] + e[1],c[2] + e[2])),
		transform(btVector3(c[0] - e[0],c[1] + e[1],c[2] + e[2])) };
	static const int d[] = {
		1,0,3,2,
		4,5,6,7,
		4,7,3,0,
		6,5,1,2,
		7,6,2,3,
		5,4,0,1
	};

	for (int i = 0; i < (sizeof(d) / sizeof(d[0]));) {
		const btVector4	p[] = {
			x[d[i++]],
			x[d[i++]],
			x[d[i++]],
			x[d[i++]]
		};

		clipDraw<4, WriteOCL>(p, ocarea);
	}
}

bool OcclusionBuffer::queryOccluder(const btVector3& c, const btVector3& e) {
	const btVector4	x[] = {
		transform(btVector3(c[0] - e[0],c[1] - e[1],c[2] - e[2])),
		transform(btVector3(c[0] + e[0],c[1] - e[1],c[2] - e[2])),
		transform(btVector3(c[0] + e[0],c[1] + e[1],c[2] - e[2])),
		transform(btVector3(c[0] - e[0],c[1] + e[1],c[2] - e[2])),
		transform(btVector3(c[0] - e[0],c[1] - e[1],c[2] + e[2])),
		transform(btVector3(c[0] + e[0],c[1] - e[1],c[2] + e[2])),
		transform(btVector3(c[0] + e[0],c[1] + e[1],c[2] + e[2])),
		transform(btVector3(c[0] - e[0],c[1] + e[1],c[2] + e[2]))
	};

	for (int i = 0; i < 8; ++i) {
		if ((x[i][2] + x[i][3]) <= 0) return(true);
	}

	static const int d[] = {
		1,0,3,2,
		4,5,6,7,
		4,7,3,0,
		6,5,1,2,
		7,6,2,3,
		5,4,0,1
	};

	for (int i = 0; i < (sizeof(d) / sizeof(d[0]));) {
		const btVector4	p[] = {
			x[d[i++]],
			x[d[i++]],
			x[d[i++]],
			x[d[i++]]
		};

		if (clipDraw<4, QueryOCL>(p, qrarea)) return(true);
	}

	return(false);
}
