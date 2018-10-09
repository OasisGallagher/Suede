#pragma once
#include "mesh.h"

enum VertexAttrib {
	/**
	* @warning must be 0, or we must set the location of _MatrixTextureBufferOffset defined in suede.inc explicitly for GL::VertexAttribI1i() usage.
	* @see https://stackoverflow.com/questions/28818997/how-to-use-glvertexattrib
	*/
	VertexAttribMatrixTextureBufferOffset,

	VertexAttribPosition,
	VertexAttribTexCoord0,
	VertexAttribNormal = VertexAttribTexCoord0 + MeshAttribute::TexCoordsCount,
	VertexAttribTangent,
	VertexAttribBoneIndexes,
	VertexAttribBoneWeights,

	VertexAttribInstanceColor,
	VertexAttribInstanceGeometry,
};
