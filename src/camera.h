#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "vector3.h"
#include <algorithm>

class Camera {
	Vector3 focus, dir, up, screenCenter;
	float d, f, unit, lenR;
	int H, W;
public:
	Camera(Vector3, Vector3, float, float, float, float, int, int);
	std::pair<Vector3, Vector3> getRay(int, int); 
};

#endif
