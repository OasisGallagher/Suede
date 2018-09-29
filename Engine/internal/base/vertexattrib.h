#pragma once

enum VertexAttrib {
	VertexAttribPosition,
	VertexAttribTexCoord,
	VertexAttribNormal,
	VertexAttribTangent,
	VertexAttribBoneIndexes,
	VertexAttribBoneWeights,

	VertexAttribInstanceColor,
	VertexAttribInstanceGeometry,

	/**
	 * @warning match the location of _MatrixTextureBufferOffset defined in suede.inc.
	 */
	VertexAttribMatrixOffset,
};
