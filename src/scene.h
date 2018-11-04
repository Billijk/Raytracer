#ifndef _SCENE_H_
#define _SCENE_H_

#include "ray.h"
#include "object.h"
#include "camera.h"
#include "vector3.h"
#include "kdtree.h"
#include "lib/tinyxml2.h"
#include <vector>
#include <string>

class Object;
class Ray;

class Scene {
	int H, W;
	std::vector<Object*> lights;
	Camera* camera = NULL;
	Color backgroundColor;
	KDTree* kdtree = NULL;

	void parseCamera(tinyxml2::XMLElement*);
	void parseLightSources(tinyxml2::XMLElement*);
	void parseObjects(tinyxml2::XMLElement*, bool);
	void parseObjFiles(std::string, Vector3, float, Material*, bool);
	// 以上两个函数，以及下面readSceneFile中的最后一个参数代表
	// 是否是预览模式。在预览模式中，复杂形体会用包围盒代替，
	// 并且忽略所有的反射和折射
public:
	Scene() {}
	~Scene();
	int getH() { return H; }
	int getW() { return W; }
	Color getBackgroundColor()  { return backgroundColor; }
	void readSceneFile(std::string, bool preview = false);
	Ray* getRay(int, int);

	friend class Ray;
};

#endif
