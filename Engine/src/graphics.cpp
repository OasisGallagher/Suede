#include "mesh.h"
#include "world.h"
#include "api/gl.h"
#include "graphics.h"
#include "resources.h"
#include "variables.h"
#include "tools/math2.h"

static void DrawSubMeshes(Mesh mesh) {
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		GLUtil::DrawElementsBaseVertex(mesh->GetTopology(), mesh->GetSubMesh(i)->GetTriangleBias());
	}
}

/**
 * [0, 1] to [-1, 1].
 */
static glm::vec3 RectCoordToGLSpace(const glm::vec2& coord) {
	return glm::vec3(coord * glm::vec2(2) - glm::vec2(1), 0);
}

static void CreateMeshAttributeFromContentRect(MeshAttribute& attribute, const Rect& rect) {
	attribute.topology = MeshTopologyTriangleStripe;

	glm::vec3 positions[] = {
		RectCoordToGLSpace(rect.GetLeftBottom()),
		RectCoordToGLSpace(rect.GetRightBottom()),
		RectCoordToGLSpace(rect.GetLeftTop()),
		RectCoordToGLSpace(rect.GetRightTop())
	};

	attribute.positions.assign(positions, positions + CountOf(positions));

	glm::vec2 texCoords[] = {
		glm::vec2(0.f, 0.f),
		glm::vec2(1.f, 0.f),
		glm::vec2(0.f, 1.f),
		glm::vec2(1.f, 1.f),
	};

	attribute.texCoords.assign(texCoords, texCoords + CountOf(texCoords));

	int indexes[] = { 0, 1, 2, 3 };
	attribute.indexes.assign(indexes, indexes + CountOf(indexes));
}

void Graphics::Blit(RenderTexture src, RenderTexture dest, Material material) {
	MeshAttribute attribute;
	CreateMeshAttributeFromContentRect(attribute, src->GetContentRect());

	Mesh mesh = NewMesh();
	mesh->SetAttribute(attribute);

	SubMesh subMesh = NewSubMesh();
	TriangleBias base{ attribute.indexes.size() };
	subMesh->SetTriangleBias(base);

	mesh->AddSubMesh(subMesh);

	if (!dest) { dest = WorldInstance()->GetScreenRenderTarget(); }

	// TODO: VIEWPORT RECT.
	dest->BindWrite(Rect(0, 0, 1, 1));
	material->SetTexture(Variables::mainTexture, src);
	Draw(mesh, material);
	dest->Unbind();
}

void Graphics::Draw(Mesh mesh, Material material) {
	mesh->Bind();
	int pass = material->GetPass();
	if (pass >= 0) {
		material->Bind(pass);
		DrawSubMeshes(mesh);
		material->Unbind();
	}
	else {
		for (pass = 0; pass < material->GetPassCount(); ++pass) {
			material->Bind(pass);
			DrawSubMeshes(mesh);
			material->Unbind();
		}
	}

	mesh->Unbind();
}
