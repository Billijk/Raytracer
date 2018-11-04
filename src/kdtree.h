#ifndef _KDTREE_H_
#define _KDTREE_H_

#include "object.h"
#include <vector>
#include <algorithm>

const extern float EPS;

class KDTree;

class TreeNode {
	std::vector<Object*> objs;
	float xl, xh, yl, yh, zl, zh; // boundaries
	float split;
	int dep;
	TreeNode *lchild, *rchild;

	// 检查射线与某一平面的交点是否在盒上
	bool checkIntersection(Vector3, Vector3, int, float&, float&, float&);

	void calcBoundaries(const float, float&, float&,
			float&, float&, float&, float&, bool);
public:
	TreeNode(float, float, float, float, float, float, int);
	~TreeNode();
	static int nodecnt;

	void addObject(Object* obj) { objs.push_back(obj); }
	void createChildren(float);
	float findBestSplitPos();
	int collideWithBox(Vector3, Vector3, float&, float&);	// 返回碰撞点的个数
	bool inBox(Vector3);	// 判断点是否在盒内
	// 返回最近的碰撞点(obj, point)，或者(NULL, (0, 0, 0))
	std::pair<Object*, Vector3> collideWithObj(Vector3, Vector3);

	int getDepth() { return dep; }
	TreeNode* getLChild() { return lchild; }
	TreeNode* getRChild() { return rchild; }

	friend class KDTree;
};

class KDTree {
	TreeNode* root;
public:
	KDTree(float, float, float, float, float, float);
	~KDTree();
	void buildTree(TreeNode*);
	std::pair<Object*, Vector3> traverse(TreeNode*, Vector3, Vector3);

	TreeNode* getRoot() { return root; }
};

#endif
