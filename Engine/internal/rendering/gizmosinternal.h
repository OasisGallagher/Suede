#pragma once
#include "gizmos.h"

#include <vector>

#include "mesh.h"
#include "material.h"

class GizmosInternal {
public:
	GizmosInternal();

public:
	void Flush();

	Matrix4 GetMatrix() { return matrix_; }
	void SetMatrix(const Matrix4& value) { matrix_ = value; }

	Color GetColor() { return color_; }
	void SetColor(const Color& value) { color_ = value; }
	
	void DrawLines(const Vector3* points, uint npoints);
	void DrawLines(const Vector3* points, uint npoints, const uint* indexes, uint nindexes);

	void DrawLineStripe(const Vector3* points, uint npoints);
	void DrawLineStripe(const Vector3* points, uint npoints, const uint* indexes, uint nindexes);

	void DrawSphere(const Vector3& center, float radius);
	void DrawCuboid(const Vector3& center, const Vector3& size);

	void DrawWireSphere(const Vector3& center, float radius);
	void DrawWireCuboid(const Vector3& center, const Vector3& size);

private:
	struct Batch {
		MeshTopology topology;

		bool wireframe;
		Color color;
		ref_ptr<Material> material;

		std::vector<uint> indexes;
		std::vector<Vector3> points;
	};

private:
	void OnFrameLeave() { Flush(); }

	Batch& GetBatch(MeshTopology topology, bool wireframe, Material* material);
	bool IsBatchable(const Batch& ref, MeshTopology topology, bool wireframe, Material* material);

	void FillBatch(Batch& b, const Vector3* points, uint npoints);
	void FillBatch(Batch& b, const Vector3* points, uint npoints, const uint* indexes, uint nindexes);

	void AddSphereBatch(const Vector3& center, float radius, bool wireframe);
	void AddCuboidBatch(const Vector3& center, const Vector3& size, bool wireframe);

	void DrawGizmos(const Batch& b);

private:
	ref_ptr<Mesh> mesh_;
	ref_ptr<Material> lineMaterial_;

	Color color_;
	Matrix4 matrix_;
	std::vector<Batch> batches_;
};