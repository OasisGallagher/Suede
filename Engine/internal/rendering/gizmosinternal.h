#pragma once
#include "gizmos.h"

#include <vector>

#include "mesh.h"
#include "material.h"
#include "internal/engine/subsysteminternal.h"

class Graphics;
class GizmosInternal : public SubsystemInternal {
public:
	GizmosInternal(Graphics* graphics);
	~GizmosInternal();

public:
	void Awake();
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

	void DrawArrow(const Vector3& from, const Vector3& to);

private:
	struct Batch {
		MeshTopology topology;

		Color color;
		ref_ptr<Material> material;

		std::vector<uint> indexes;
		std::vector<Vector3> points;
	};

private:
	void Update(float deltaTime) { Flush(); }

	Batch& GetBatch(MeshTopology topology, Material* material);
	bool IsBatchable(const Batch& ref, MeshTopology topology, Material* material);

	void FillBatch(Batch& b, const Vector3* points, uint npoints);
	void FillBatch(Batch& b, const Vector3* points, uint npoints, const uint* indexes, uint nindexes);

	void AddSphereBatch(const Vector3& center, float radius, bool wireframe);
	void AddCuboidBatch(const Vector3& center, const Vector3& size, bool wireframe);

	Vector3 GetPerpendicular(const Vector3 &v);
	void AddConeBatch(const Vector3 &direction, const Vector3 &apex, float coneHeight, float coneRadius, uint slices);

	void DrawGizmos(const Batch& b);

private:
	Graphics* graphics_;

	ref_ptr<Mesh> mesh_;
	ref_ptr<Material> lineMaterial_;

	Color color_;
	Matrix4 matrix_;
	std::vector<Batch> batches_;
};
