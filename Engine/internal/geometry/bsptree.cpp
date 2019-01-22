//#include "plane.h"
//#include "bsptree.h"
//#include "polygon.h"
//#include "memory/memory.h"
//#include "geometryutility.h"
//
//#define MINIMUM_RELATION	.8f
//#define MIN_RELATION_SCALE	2.f
//
//BSPTreeNode::BSPTreeNode() : tree(nullptr)
//	, lchild(nullptr), rchild(nullptr), divider(nullptr) {
//}
//
//void BSPTree::Create(const Polygon* polygons, uint count) {
//	if (root_ != nullptr) {
//		Destroy();
//	}
//
//	std::vector<const Polygon*> pointers(count);
//	for (uint i = 0; i < count; ++i) {
//		pointers[i] = polygons + i;
//	}
//
//	root_ = CreateNode();
//	CreateSubtree(root_, &pointers[0], count);
//}
//
//const Polygon* BSPTree::FindDivider(const Polygon** polygons, uint count) {
//	float minRelation = MINIMUM_RELATION, bestRelation = 0;
//	int bestPolygon = -1;
//	uint leastSplits = UINT_MAX;
//
//	std::vector<bool> used(count, false);
//
//	for (; bestPolygon == -1; ) {
//		for (int i = 0; i < count; ++i) {
//			if (used[i]) { continue; }
//			uint npositive = 0, nnegative = 0, nspanning = 0;
//
//			for (int j = 0; j < count; ++j) {
//				if (i == j) { continue; }
//				Plane plane(polygons[i]->points);
//				Side side = GeometryUtility::CalculateSide(plane, polygons[j]->points, 3);
//				if (side == SideInfront) { ++npositive; }
//				else if (side == SideBehind) { ++nnegative; }
//				else if (side == SideSpanning) { ++nspanning; }
//			}
//
//			float relation;
//			if (npositive < nnegative) {
//				relation = float(npositive) / nnegative;
//			}
//			else {
//				relation = float(nnegative) / npositive;
//			}
//
//			if (relation > minRelation
//				&& (nspanning < leastSplits || (nspanning == leastSplits && relation > bestRelation))) {
//				bestPolygon = i;
//				leastSplits = nspanning;
//				bestRelation = relation;
//				used[i] = true;
//			}
//		}
//
//		minRelation /= MIN_RELATION_SCALE;
//	}
//
//	return polygons[bestPolygon];
//}
//
//void BSPTree::CreateSubtree(BSPTreeNode* node, const Polygon* polygons[], uint count) {
//	if (IsConvexSet(polygons, count)) {
//		node->polygons.assign(polygons, polygons + count);
//		return;
//	}
//
//	node->divider = FindDivider(polygons, count);
//	Plane plane(node->divider->points);
//	std::vector<const Polygon*> positives, negatives;
//
//	for (uint i = 0; i < count; ++i) {
//		Side side = GeometryUtility::CalculateSide(plane, polygons[i]->points, polygons[i]->npoints);
//		if (side == SideInfront) {
//			positives.push_back(polygons[i]);
//		}
//		else if (side == SideBehind) {
//			negatives.push_back(polygons[i]);
//		}
//		else if (side == SideSpanning) {
//			// split polygon into front and back set.
//			// merge front into positives.
//			// merge back into negatives.
//		}
//	}
//
//	if (!positives.empty()) {
//		node->lchild = CreateNode();
//		CreateSubtree(node->lchild, &positives[0], positives.size());
//	}
//
//	if (!negatives.empty()) {
//		node->rchild = CreateNode();
//		CreateSubtree(node->rchild, &negatives[0], negatives.size());
//	}
//}
//
//void BSPTree::DrawSubtree(BSPTreeNode* node, const glm::vec3 & viewer) {
//	if (node == nullptr) { return; }
//
//	if (node->IsLeaf()) {
//		// draw polygons.
//		return;
//	}
//
//	Plane plane(node->divider->points);
//	Side side = GeometryUtility::CalculateSide(plane, &viewer, 1);
//
//	// if the viewer is in the right subtree draw that tree before the left.
//	if (side == SideInfront || side == SideCoinciding) {
//		DrawSubtree(node->rchild, viewer);
//		DrawSubtree(node->lchild, viewer);
//	}
//	else { // otherwise, draw the left first.
//		DrawSubtree(node->lchild, viewer);
//		DrawSubtree(node->rchild, viewer);
//	}
//}
//
//BSPTreeNode* BSPTree::CreateNode() {
//	BSPTreeNode* answer = MEMORY_NEW(BSPTreeNode);
//	answer->tree = this;
//	return answer;
//}
//
//bool BSPTree::InFront(const Polygon* first, const Polygon* second) {
//	Plane plane(first->points);
//	for (uint i = 0; i < second->npoints; ++i) {
//		if (glm::dot(plane.GetNormal(), second->points[i]) + plane.GetDistance() <= 0) {
//			return false;
//		}
//	}
//
//	return true;
//}
//
//bool BSPTree::IsConvexSet(const Polygon** polygons, uint count) {
//	for (uint i = 0; i < count; ++i) {
//		for (uint j = 0; j < count; ++j) {
//			if (i != j && !InFront(polygons[i], polygons[j])) {
//				return false;
//			}
//		}
//	}
//
//	return true;
//}
//
//bool BSPTree::IsInsideFrustum(Plane* planes, uint nplanes, const Polygon* polygon) {
//	for (uint i = 0; i < polygon->npoints; ++i) {
//		bool inside = true;
//		for (uint j = 0; j < nplanes; ++j) {
//			if (GeometryUtility::CalculateSide(planes[j], polygon->points + i, 1) != SideInfront) {
//				inside = false;
//			}
//		}
//
//		if (inside) { return true; }
//	}
//
//	return false;
//}
//
//void BSPTree::RenderPortal(const Polygon** portals, uint nportal, const Plane* plane, uint nplanes) {
//	for (uint i = 0; i < nportal; ++i) {
//		
//	}
//}
//
//void BSPTree::PlacePortals(const Polygon* polygon, BSPTreeNode* node) {
//	if (node->IsLeaf()) {
//		bool clipped = false;
//
//		for (uint i = 0; i < node->polygons.size(); ++i) {
//			Plane plane(node->polygons[i]->points);
//			if (GeometryUtility::CalculateSide(plane, polygon->points, polygon->npoints) == SideSpanning) {
//				clipped = true;
//
//				std::vector<glm::vec3> negative, positive;
//				ClipPolygon(negative, positive, plane, node->polygons[i]);
//
//				Polygon np(&negative[0], negative.size()), pp(&positive[0], positive.size());
//				PlacePortals(&pp, root_);
//				PlacePortals(&np, root_);
//			}
//		}
//
//		if (!clipped) {
//			// remove the parts of the portal polygon that coincide with other 
//			// polygons in this node.
//			// add this node to the set of connected nodes in this portal polygon.
//		}
//	}
//}
//
//void BSPTree::ClipPolygon(std::vector<glm::vec3>& negative, std::vector<glm::vec3>& positive, const Plane& clipper, const Polygon* polygon) {
//	typedef std::pair<glm::vec3, glm::vec3> Edge;
//	for (uint i = 1; i <= polygon->npoints; ++i) {
//		Edge edge(polygon->points[i - 1], polygon->points[i % polygon->npoints]);
//		Side side0 = GeometryUtility::CalculateSide(clipper, &edge.first, 1);
//		Side side1 = GeometryUtility::CalculateSide(clipper, &edge.second, 1);
//
//		if (side0 != side1 && side0 != SideCoinciding && side1 != SideCoinciding) {
//			glm::vec3 intersection;
//			if (!GeometryUtility::GetIntersection(intersection, clipper, edge.first, edge.second)) {
//				Debug::LogError("logic error");
//			}
//
//			if (side0 == SideInfront) {
//				positive.push_back(edge.first);
//				positive.push_back(intersection);
//
//				negative.push_back(intersection);
//				negative.push_back(edge.second);
//			}
//			else if (side0 == SideBehind) {
//				negative.push_back(edge.first);
//				negative.push_back(intersection);
//
//				positive.push_back(intersection);
//				positive.push_back(edge.second);
//			}
//		}
//		else {
//			if (side0 == SideInfront || side1 == SideInfront || (side0 == SideCoinciding && side1 == SideCoinciding)) {
//				positive.push_back(edge.first);
//				positive.push_back(edge.second);
//			}
//
//			if (side0 == SideBehind || side1 == SideBehind) {
//				negative.push_back(edge.first);
//				negative.push_back(edge.second);
//			}
//		}
//	}
//}