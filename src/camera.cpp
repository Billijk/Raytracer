#include "camera.h"
#include <cmath>

using namespace std;

Camera::Camera (Vector3 F, Vector3 D,
		float _d, float _f, float _u, float r, int h, int w):
	focus(F), dir(D.normalized()),
	d(_d), f(_f), unit(_u), lenR(r), H(h), W(w) {
		up = Vector3(0, -D.z, D.y).normalized();
		screenCenter = F + D * d;
}

pair<Vector3, Vector3> Camera::getRay(int x, int y) {
	Vector3 right = up.cross(dir).normalized();
	// 计算幕布上的坐标位置
	Vector3 screenPoint = screenCenter + 
		up * ((y  -  H / 2 ) * unit) + 
		right * ((x  - W / 2) * unit);

	// 计算原点位置
	float dr = sqrt((float) rand() / (float) RAND_MAX * lenR);
	float dtheta = (float) rand() / (float) RAND_MAX * 2 * 3.14159;
	float dx = dr * cos(dtheta), dy = dr * sin(dtheta);
	Vector3 origin = focus + right * dx + up * dy;

	// 计算焦平面上的位置(光圈较小，近似认为打在平面上)
	Vector3 fpos = focus + (screenPoint - focus).normalized() * f;

	return pair<Vector3, Vector3>(origin, (fpos - origin).normalized());
}
