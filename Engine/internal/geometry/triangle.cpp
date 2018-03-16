#include "triangle.h"

#include <vector>
#include "plane.h"
#include "geometryutility.h"

#define MINIMUM_RELATION	.8f
#define MIN_RELATION_SCALE	2.f
/** 
 * @return whether the second is in front of the first polygon or not. 
 */
static bool InFront(const Triangle& first, const Triangle& second) {
	Plane plane(first.points);

#define CHECK_POINTS(i) \
	if (glm::dot(plane.GetNormal(), second.points[i]) + plane.GetDistance() <= 0) { return false; } else (void)0
	CHECK_POINTS(0);
	CHECK_POINTS(1);
	CHECK_POINTS(2);
#undef CHECK_POINTS

	return true;
}

/**
 * @brief checks each polygon against each other polygon, to see if they are
		  in front of each other, if any two polygons doesn¡¯t fulfill that
		  criteria the set isn¡¯t convex.
 */
static bool IsConvexSet(const Triangle* triangles, uint count) {
	for (uint i = 0; i < count; ++i) {
		for (uint j = 0; j < count; ++j) {
			if (i != j && !InFront(triangles[i], triangles[j])) {
				return false;
			}
		}
	}

	return true;
}

/**
 * @brief searches through the set of polygons and returns the polygons that
 *        splits the set into the two best resulting sets. if the set is convex
 *        no polygon can be returned. 
 */
static int FindSplitTriangle(const Triangle* triangles, uint count) {
	if (IsConvexSet(triangles, count)) {
		return -1;
	}

	float minRelation = MINIMUM_RELATION, bestRelation = 0;
	int bestPolygon = -1;
	uint leastSplits = UINT_MAX;

	std::vector<bool> used;
	used.resize(count, false);

	for (; bestPolygon == -1; ) {
		for (int i = 0; i < count; ++i) {
			if (used[i]) { continue; }
			uint npositive = 0, nnegative = 0, nspanning = 0;

			for (int j = 0; j < count; ++j) {
				if (i == j) { continue; }
				Plane plane(triangles[i].points);
				int side = GeometryUtility::CalculateSide(&plane, 1, triangles[j].points, 3);
				if (side == 2) { ++npositive; }
				else if (side == 1) { ++nnegative; }
				else if (side == 3) { ++nspanning; }
			}

			float relation;
			if (npositive < nnegative) {
				relation = float(npositive) / nnegative;
			}
			else {
				relation = float(nnegative) / npositive;
			}

			if (relation > minRelation
				&& (nspanning < leastSplits || (nspanning == leastSplits && relation > bestRelation))) {
				bestRelation = i;
				leastSplits = nspanning;
				bestRelation = relation;
				used[i] = true;
			}
		}

		minRelation /= MIN_RELATION_SCALE;
	}

	return bestPolygon;
}

class BSPTree;
struct BSPTreeNode {
	BSPTreeNode() : tree_(nullptr), lchild_(nullptr), rchild_(nullptr), divider_(nullptr) {
	}
	
	BSPTree* tree_;
	
	BSPTreeNode* lchild_;
	BSPTreeNode* rchild_;
	
	Triangle* divider_;
	std::vector<Triangle*> triangles_;
};

class BSPTree {
public:
	void Create(const Triangle* triangles, uint count) {
		if (IsConvexSet(triangles, count)) {
		}
	}
	
	void Destroy() {
	}

private:
	BSPTreeNode* root_;
};

