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
	 * @warning Match location of suede.inc -> c_matrixBufferOffset.
	 */
	VertexAttribMatrixOffset,
};
