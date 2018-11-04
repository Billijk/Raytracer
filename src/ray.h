#ifndef _RAY_H_
#define _RAY_H_

#include "scene.h"
#include "vector3.h"
#include "color.h"
#include "object.h"
#include <algorithm>

extern const float EPS;

class Object;
class Scene;

class Ray {
	bool insideObj;
	Vector3 startPoint, direction;
	std::pair<Object*, Vector3> detectCollide(Scene*);
	
	Color calcReflection(Scene*, Object*, Vector3, int, bool);
	Color calcRefraction(Scene*, Object*, Vector3, int, bool);
	Color calcDiffusion(Scene*, Object*, Object*, Vector3);

	Ray reflectRay(Vector3, Vector3);
	Ray refractRay(Vector3, Vector3, float);
public:
	Ray(Vector3, Vector3, bool inside = 0);
	Vector3 getStartPoint() {return startPoint;}
	Vector3 getDirection() {return direction;}
	Color rayTracing(Scene*, int, bool pt = 1);
};

#endif
