#include "kdtree.h"
#include <map>
#include <set>

using namespace std;

int TreeNode::nodecnt = 0;

TreeNode::TreeNode(float _xl, float _xh, float _yl, 
		float _yh, float _zl, float _zh, int _d) :
	xl(_xl), xh(_xh), yl(_yl), yh(_yh), zl(_zl), zh(_zh), dep(_d) {
		lchild = rchild = NULL;
		nodecnt ++;
}

TreeNode::~TreeNode() {
	if (lchild) {
		delete lchild;
		lchild = NULL;
	}
	if (rchild) {
		delete rchild;
		rchild = NULL;
	}
}

bool TreeNode::checkIntersection(Vector3 s, Vector3 d, int axis,
		float& x, float& y, float& z) {
	if (axis == 0) {
		float t = (x - s.x) / d.x;
		y = s.y + t * d.y;
		z = s.z + t * d.z;
		return (t >= 0 && y >= yl && y <= yh && z >= zl && z <= zh);
	}
	else if (axis == 1) {
		float t = (y - s.y) / d.y;
		x = s.x + t * d.x;
		z = s.z + t * d.z;
		return (t >= 0 && x >= xl && x <= xh && z >= zl && z <= zh);
	}
	else if (axis == 2) {
		float t = (z - s.z) / d.z;
		x = s.x + t * d.x;
		y = s.y + t * d.y;
		return (t >= 0 && x >= xl && x <= xh && y >= yl && y <= yh);
	}
	return false;
}

void TreeNode::calcBoundaries(const float p, float& _xl, float& _xh,
			float& _yl, float& _yh, float& _zl, float& _zh, bool low) {
	_xl = xl, _xh = xh;
	_yl = yl, _yh = yh;
	_zl = zl, _zh = zh;
	if (dep % 3 == 0) {
		if (low == 0) _xl = xl, _xh = p;
		else _xl = p, _xh = xh;
	}
	else if (dep % 3 == 1) {
		if (low == 0) _yl = yl, _yh = p;
		else _yl = p, _yh = yh;
	}
	else if (dep % 3 == 2) {
		if (low == 0) _zl = zl, _zh = p;
		else _zl = p, _zh = zh;
	}
}

void TreeNode::createChildren(float splitPos) {
	float xl1, xl2, xh1, xh2;
	float yl1, yl2, yh1, yh2;
	float zl1, zl2, zh1, zh2;
	calcBoundaries(splitPos, xl1, xh1, yl1, yh1, zl1, zh1, 0);
	calcBoundaries(splitPos, xl2, xh2, yl2, yh2, zl2, zh2, 1);
	lchild = new TreeNode(xl1, xh1, yl1, yh1, zl1, zh1, dep + 1);
	rchild = new TreeNode(xl2, xh2, yl2, yh2, zl2, zh2, dep + 1);
	split = splitPos;
}

float TreeNode::findBestSplitPos() {
	float bl, bh;
	if (dep % 3 == 0) bl = xl, bh = xh;
	else if (dep % 3 == 1) bl = yl, bh = yh;
	else bl = zl, bh = zh;
	map<float, Object*> sorted_objs;
	set<float> positions;
	for (auto& obj: objs) {
		float l = obj -> calcExtreme(dep % 3, 0);
		float h = obj -> calcExtreme(dep % 3, 1);
		if (l - EPS >= bl) positions.insert(l - EPS);
		if (l + EPS >= bl) positions.insert(l + EPS);
		if (h - EPS <= bh) positions.insert(h - EPS);
		if (h + EPS <= bh) positions.insert(h + EPS);
		sorted_objs.insert(pair<float, Object*>(h, obj));
	}

	float xl1, xl2, xh1, xh2;
	float yl1, yl2, yh1, yh2;
	float zl1, zl2, zh1, zh2;
	float bestPos = *(positions.begin()), bestCost = 1e100;
	int lcnt = 0, rcnt = sorted_objs.size();
	auto j = sorted_objs.begin();
	for (auto i = positions.begin(); i != positions.end(); ++ i) {
		calcBoundaries(*i, xl1, xh1, yl1, yh1, zl1, zh1, 0);
		calcBoundaries(*i, xl2, xh2, yl2, yh2, zl2, zh2, 1);
		while (j != sorted_objs.end() && (*j).first <= (*i))
			lcnt ++, rcnt --, j ++;
		float dx1 = xh1 - xl1, dy1 = yh1 - yl1, dz1 = zh1 - zl1;
		float dx2 = xh2 - xl2, dy2 = yh2 - yl2, dz2 = zh2 - zl2;
		float cost = (dx1 * dy1 + dy1 * dz1 + dx1 * dz1) * lcnt + 
			(dx2 * dy2 + dy2 * dz2 + dx2 * dz2) * rcnt;
		if (cost < bestCost) bestCost = cost, bestPos = *i;
	}
	return bestPos;
}

int TreeNode::collideWithBox(Vector3 startPoint, Vector3 direction, float& p1, float& p2) {
	float coords[9] = {0};
	int i = 0;
	if (checkIntersection(startPoint, direction, 0, coords[i] = xl, coords[i + 1], coords[i + 2])) i += 3;
	if (checkIntersection(startPoint, direction, 0, coords[i] = xh, coords[i + 1], coords[i + 2])) i += 3;
	if (checkIntersection(startPoint, direction, 1, coords[i], coords[i + 1] = yl, coords[i + 2])) i += 3;
	if (checkIntersection(startPoint, direction, 1, coords[i], coords[i + 1] = yh, coords[i + 2])) i += 3;
	if (checkIntersection(startPoint, direction, 2, coords[i], coords[i + 1], coords[i + 2] = zl)) i += 3;
	if (checkIntersection(startPoint, direction, 2, coords[i], coords[i + 1], coords[i + 2] = zh)) i += 3;

	if (dep % 3 == 0) {
		if (i / 3 == 1) p1 = coords[0];
		else {
			if (fabs(coords[0] - startPoint.x) < fabs(coords[3] - startPoint.x))
				p1 = coords[0], p2 = coords[3];
			else p1 = coords[3], p2 = coords[0];
		}
	}
	else if (dep % 3 == 1) {
		if (i / 3 == 1) p1 = coords[1];
		else {
			if (fabs(coords[1] - startPoint.y) < fabs(coords[4] - startPoint.y))
				p1 = coords[1], p2 = coords[4];
			else p1 = coords[4], p2 = coords[1];
		}
	}
	else if (dep % 3 == 2) {
		if (i / 3 == 1) p1 = coords[2];
		else {
			if (fabs(coords[2] - startPoint.z) < fabs(coords[5] - startPoint.z))
				p1 = coords[2], p2 = coords[5];
			else p1 = coords[5], p2 = coords[2];
		}
	}
	return i / 3;
}

bool TreeNode::inBox(Vector3 p) {
	return xl <= p.x && p.x <= xh
		&& yl <= p.y && p.y <= yh
		&& zl <= p.z && p.z <= zh;
}

pair<Object*, Vector3> TreeNode::collideWithObj(Vector3 startPoint, Vector3 direction) {
	map<float, pair<Object*, Vector3>> collided;
	for (auto &obj: objs) {
		Vector3 hitPoint = obj -> collideWith(startPoint, direction);
		if (!hitPoint.isZeroVector() && inBox(hitPoint)) {
			pair<Object*, Vector3> hit(obj, hitPoint);
			float dist = hitPoint.distance(startPoint);
			collided.insert(pair<float, pair<Object*, Vector3>>(dist, hit));
		}
	}
	if (collided.empty()) return make_pair<Object*, Vector3>(NULL, Vector3());
	else return (*collided.cbegin()).second;
}

KDTree::KDTree(float xl, float xh, float yl, float yh, float zl, float zh) {
	root = new TreeNode(xl, xh, yl, yh, zl, zh, 0);
}

KDTree::~KDTree() {
	for (auto& i: root -> objs) {
		delete i -> getMaterial();
		delete i;
	}
	delete root;
}

void KDTree::buildTree(TreeNode* node) {
	if ((node -> objs).size() <= 40) return;
	float splitPos = node -> findBestSplitPos();
	node -> createChildren(splitPos);
	for (auto& obj: node -> objs) {
		if (obj -> intersectWithBox(node -> lchild -> xl,
					node -> lchild -> xh,
					node -> lchild -> yl,
					node -> lchild -> yh,
					node -> lchild -> zl,
					node -> lchild -> zh)) {
			node -> lchild -> addObject(obj);
		}
		if (obj -> intersectWithBox(node -> rchild -> xl,
					node -> rchild -> xh,
					node -> rchild -> yl,
					node -> rchild -> yh,
					node -> rchild -> zl,
					node -> rchild -> zh)) {
			node -> rchild -> addObject(obj);
		}
	}
	buildTree(node -> lchild);
	buildTree(node -> rchild);
}

pair<Object*, Vector3> KDTree::traverse(TreeNode* node, Vector3 startPoint, Vector3 direction) {
	// 盒中无物体
	if ((node -> objs).size() == 0)
		return make_pair<Object*, Vector3>(NULL, Vector3());
	// 无子节点
	if (node -> lchild == 0 && node -> rchild == 0) {
		auto hit = node -> collideWithObj(startPoint, direction);
		if (hit.first) return hit;
		else return make_pair<Object*, Vector3>(NULL, Vector3());
	}

	float p1, p2, split = node -> split;
	int dep = node -> dep;
	int c = node -> collideWithBox(startPoint, direction, p1, p2);
	
	if (!c) return make_pair<Object*, Vector3>(NULL, Vector3());
	if (c == 1) { // 起点在盒中
		p2 = p1;
		if (dep % 3 == 0) p1 = startPoint.x;
		else if (dep % 3 == 1) p1 = startPoint.y;
		else if (dep % 3 == 2) p1 = startPoint.z;
	}

	if (p1 >= split && p2 >= split) return traverse(node -> rchild, startPoint, direction);
	else if (p1 <= split && p2 >= split) {
		auto hit = traverse(node -> lchild, startPoint, direction);
		if (hit.first == NULL) return traverse(node -> rchild, startPoint, direction);
		else return hit;
	}
	else if (p1 >= split && p2 <= split) {
		auto hit = traverse(node -> rchild, startPoint, direction);
		if (hit.first == NULL) return traverse(node -> lchild, startPoint, direction);
		else return hit;
	}
	else if (p1 <= split && p2 <= split) return traverse(node -> lchild, startPoint, direction);
	return make_pair<Object*, Vector3>(NULL, Vector3());
}
