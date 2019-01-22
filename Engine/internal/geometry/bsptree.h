//#include <vector>
//#include <glm/glm.hpp>
//
//#include "types.h"
//
//class BSPTree;
//struct Polygon;
//
//struct BSPTreeNode {
//	BSPTreeNode();
//	bool IsLeaf() const { return lchild == nullptr && rchild == nullptr; }
//
//	BSPTree* tree;
//
//	BSPTreeNode* lchild;
//	BSPTreeNode* rchild;
//
//	const Polygon* divider;
//	std::vector<Polygon*> polygons;
//};
//
//class BSPTree {
//public:
//	/**
//	 * @brief create a BSP-tree out of a set of polygons.
//	 */
//	void Create(const Polygon* polygons, uint count);
//
//	/**
//	 * @brief draw the polygons contained in the node and its subtrees.
//	 */
//	void Draw(const glm::vec3& viewer) {
//		DrawSubtree(root_, viewer);
//	}
//
//	void Destroy() {
//	}
//
//private:
//	/**
//	 * @brief searches through the set of polygons and returns the polygons that
//	 *        splits the set into the two best resulting sets. the set must be convex.
//	 */
//	const Polygon* FindDivider(const Polygon** polygons, uint count);
//
//	void CreateSubtree(BSPTreeNode* node, const Polygon* polygons[], uint count);
//	void DrawSubtree(BSPTreeNode* node, const glm::vec3& viewer);
//
//	/**
//	 * @return whether the second is in front of the first polygon or not.
//	 */
//	bool InFront(const Polygon* first, const Polygon* second);
//
//	/**
//	 * @brief checks each polygon against each other polygon, to see if they are
//	 *        in front of each other, if any two polygons doesn’t fulfill that
//	 *        criteria the set isn’t convex.
//	 */
//	bool IsConvexSet(const Polygon** polygons, uint count);
//
//	/**
//	 * @brief check each points in the polygon versus each plane in the view frustum.
//	 *        if any points is on the positive side of all planes the polygon is
//	 *        counted as inside.
//	 */
//	bool IsInsideFrustum(Plane* planes, uint nplanes, const Polygon* polygon);
//
//	/**
//	 * @brief renders the polygons in a portal engine. where the world is represented
//	 *        as sectors connected by portals.
//	 */
//	void RenderPortal(const Polygon** portals, uint nportal, const Plane* plane, uint nplanes);
//
//	/**
//	 * @brief pushes a portal polygon down through the tree clipping then it needs it.
//	 *        the output of this function will be that each node contains a list of 
//	 *        portal polygons where each portal connected exactly two nodes.
//	 */
//	void PlacePortals(const Polygon* polygon, BSPTreeNode* node);
//	void ClipPolygon(std::vector<glm::vec3>& negative, std::vector<glm::vec3>& positive, const Plane& clipper, const Polygon* polygon);
//
//	BSPTreeNode* CreateNode();
//
//private:
//	BSPTreeNode* root_;
//};
