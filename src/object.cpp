#include "object.h"
#include "vector3.h"
#include "ray.h"
#include "bmp.h"
#include <cstdio>
#include <cmath>
#include <algorithm>

using namespace std;

Sphere::Sphere(Vector3 c, float r, bool l): center(c), radius(r) {
	light = l;
}
Plane::Plane(Vector3 _ul, Vector3 _dr, Vector3 _n, bool l):
	ul(_ul), dr(_dr), n(_n.normalized()) {
	light = l;
	Vector3 t = dr - ul;
	udir = (t + t.cross(n)).normalized();
	vdir = (t - t.cross(n)).normalized();
	ulen = t.dot(udir);
	vlen = t.dot(vdir);
	UAxis = Vector3(n.y, n.z, -n.x);
	VAxis = UAxis.cross(n);
}
Triangle::Triangle(Vector3 _a, Vector3 _b, Vector3 _c, 
		Vector3 _na, Vector3 _nb, Vector3 _nc, bool l):
	a(_a), b(_b), c(_c), na(_na), nb(_nb), nc(_nc) {
	n = (c - a).cross(b - a).normalized();
	light = l;
}
Triangle::Triangle(Vector3 _a, Vector3 _b, Vector3 _c, bool l): 
	a(_a), b(_b), c(_c) {
	n = (c - a).cross(b - a).normalized();
	na = nb = nc = n;
	light = l;
}

Vector3 Sphere::getNormVec(const Vector3& point) const {
	Vector3 norm = point - center;
	norm.normalize();
	return norm;
}
Vector3 Plane::getNormVec(const Vector3& point) const {
	return n;
}
Vector3 Triangle::getNormVec(const Vector3& point) const {
	Vector3 AB = b - a, AC = c - a, AP = point - a;
	float u = (AP.x * AC.y - AP.y * AC.x) / (AB.x * AC.y - AB.y * AC.x);
	float v = (AP.x * AB.y - AP.y * AB.x) / (AB.y * AC.x - AB.x * AC.y);
	return na * (1 - u - v) + nb * u + nc * v;
}

Vector3 Sphere::collideWith(const Vector3& startPoint, const Vector3& dir) const {
	float dist = center.distToLine(startPoint, dir);
	if (dist <= radius) {
		float t = dir.dot(center - startPoint);
		float d = sqrt(radius * radius - dist * dist);
		if (t - d > EPS) return (startPoint + dir * (t - d));
		else if (t + d > EPS) return (startPoint + dir * (t + d));
		else return Vector3(0, 0, 0);
	} else return Vector3(0, 0, 0);
}
Vector3 Plane::collideWith(const Vector3& startPoint, const Vector3& dir) const {
	if (fabs(n.dot(dir)) < EPS) return Vector3(0, 0, 0);
	float d = - ul.dot(n);
	float t = - (n.dot(startPoint) + d) / n.dot(dir);
	if (t < EPS) return Vector3(0, 0, 0);
	else {
		Vector3 hitpoint = dir * t + startPoint;
		Vector3 a = hitpoint - ul;
		float u = a.dot(udir);
		float v = a.dot(vdir);
		if (u < EPS || u > ulen - EPS || v < EPS || v > vlen - EPS)
			return Vector3(0, 0, 0);
		else return hitpoint;
	}
}
Vector3 Triangle::collideWith(const Vector3& startPoint, const Vector3& dir) const {
	if (fabs(n.dot(dir)) < EPS) return Vector3(0, 0, 0);
	float d = - a.dot(n);
	float t = - (n.dot(startPoint) + d) / n.dot(dir);
	if (t < EPS) return Vector3(0, 0, 0);
	else {
		Vector3 hitpoint = dir * t + startPoint;
		auto checkSameSide = [](Vector3 v1, Vector3 v2, Vector3 vt) {
			return v1.cross(v2).dot(v1.cross(vt)) >= 0; };
		if (checkSameSide(b - a, c - a, hitpoint - a) &&
				checkSameSide(c - b, a - b, hitpoint - b) &&
				checkSameSide(a - c, b - c, hitpoint - c))
			return hitpoint;
		else return Vector3(0, 0, 0);
	}
}

float Sphere::calcExtreme(int axis, bool high) const {
	if (axis == 0 && high == 0) return center.x - radius;
	if (axis == 0 && high == 1) return center.x + radius;
	if (axis == 1 && high == 0) return center.y - radius;
	if (axis == 1 && high == 1) return center.y + radius;
	if (axis == 2 && high == 0) return center.z - radius;
	if (axis == 2 && high == 1) return center.z + radius;
	return 0;
}
float Plane::calcExtreme(int axis, bool high) const {
	Vector3 ur = ul + udir * ulen;
	Vector3 dl = ul + vdir * vlen;
	if (axis == 0 && high == 0) return min(min(min(ul.x, ur.x), dl.x), dr.x);
	if (axis == 0 && high == 1) return max(max(max(ul.x, ur.x), dl.x), dr.x);
	if (axis == 1 && high == 0) return min(min(min(ul.y, ur.y), dl.y), dr.y);
	if (axis == 1 && high == 1) return max(max(max(ul.y, ur.y), dl.y), dr.y);
	if (axis == 2 && high == 0) return min(min(min(ul.z, ur.z), dl.z), dr.z);
	if (axis == 2 && high == 1) return max(max(max(ul.z, ur.z), dl.z), dr.z);
	return 0;
}
float Triangle::calcExtreme(int axis, bool high) const {
	if (axis == 0 && high == 0) return min(min(a.x, b.x), c.x);
	if (axis == 0 && high == 1) return max(max(a.x, b.x), c.x);
	if (axis == 1 && high == 0) return min(min(a.y, b.y), c.y);
	if (axis == 1 && high == 1) return max(max(a.y, b.y), c.y);
	if (axis == 2 && high == 0) return min(min(a.z, b.z), c.z);
	if (axis == 2 && high == 1) return max(max(a.z, b.z), c.z);
	return 0;
}

bool Sphere::intersectWithBox(float xl, float xh, float yl, float yh, float zl, float zh) const {
#define sqr(x) ((x) * (x))
	float dmin = 0;
	if (center.x < xl) dmin += sqr(xl - center.x);
	else if (center.x > xh) dmin += sqr(center.x - xh);
	if (center.y < yl) dmin += sqr(yl - center.y);
	else if (center.y > yh) dmin += sqr(center.y - yh);
	if (center.z < zl) dmin += sqr(zl - center.z);
	else if (center.z > zh) dmin += sqr(center.z - zh);
	return (dmin < sqr(radius));
#undef sqr
}
bool Plane::intersectWithBox(float xl, float xh, float yl, float yh, float zl, float zh) const {
	float xd = xl + (xh - xl) / 2;
	float yd = yl + (yh - yl) / 2;
	float zd = zl + (zh - zl) / 2;
	Vector3 delta = Vector3(xd, yd, zd);
	Vector3 box = Vector3(xh - xd, yh - yd, zh - zd);
	Vector3 ur = ul + udir * ulen;
	Vector3 dl = ul + vdir * vlen;

	Vector3 _ul = ul - delta, _ur = ur - delta;
	Vector3 _dl = dl - delta, _dr = dr - delta;
	float min, max;

	// Bullets 1
#define FINDMINMAX(x0, x1, x2, x3, min, max) \
	min = max = x0;   \
	if (x1 < min) min = x1;\
	if (x1 > max) max = x1;\
	if (x2 < min) min = x2;\
	if (x2 > max) max = x2;\
	if (x3 < min) min = x3;\
	if (x3 > max) max = x3;
	FINDMINMAX(_ul[0], _ur[0], _dl[0], _dr[0], min, max);
	if(min > box[0] || max < -box[0]) return 0;
	FINDMINMAX(_ul[1], _ur[1], _dl[1], _dr[0], min, max);
	if(min > box[1] || max < -box[1]) return 0;
	FINDMINMAX(_ul[2], _ur[2], _dl[2], _dr[2], min, max);
	if(min > box[2] || max < -box[2]) return 0;
#undef FINDMINMAX

	// Bullets 2
	return Object::planeBoxOverlap(n, _ul, box);

}
bool Triangle::intersectWithBox(float xl, float xh, float yl, float yh, float zl, float zh) const {
	// ref: http://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/code/tribox3.txt
	// 以盒子中心为原点新建坐标系
	float xd = xl + (xh - xl) / 2;
	float yd = yl + (yh - yl) / 2;
	float zd = zl + (zh - zl) / 2;
	Vector3 delta = Vector3(xd, yd, zd);
	Vector3 box = Vector3(xh - xd, yh - yd, zh - zd);
	Vector3 _a = a - delta, _b = b - delta, _c = c - delta;

	Vector3 AB = _b - _a, AC = _c - _a, BC = _c - _b;
	float rad, p0, p1, p2, min, max;

	// Bullets 3
#define AXISTEST_X01(x, y, fa, fb)			   \
	p0 = x*_a[1] - y*_a[2];			       	   \
	p2 = x*_c[1] - y*_c[2];			       	   \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
	rad = fa * box[1] + fb * box[2];   \
	if(min>rad || max<-rad) return 0;

#define AXISTEST_X2(x, y, fa, fb)			   \
	p0 = x*_a[1] - y*_a[2];			           \
	p1 = x*_b[1] - y*_b[2];			       	   \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * box[1] + fb * box[2];   \
	if(min>rad || max<-rad) return 0;

#define AXISTEST_Y02(x, y, fa, fb)			   \
	p0 = -x*_a[0] + y*_a[2];		      	   \
	p2 = -x*_c[0] + y*_c[2];	       	       	   \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
	rad = fa * box[0] + fb * box[2];   \
	if(min>rad || max<-rad) return 0;

#define AXISTEST_Y1(x, y, fa, fb)			   \
	p0 = -x*_a[0] + y*_a[2];		      	   \
	p1 = -x*_b[0] + y*_b[2];	     	       	   \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * box[0] + fb * box[2];   \
	if(min>rad || max<-rad) return 0;

#define AXISTEST_Z12(x, y, fa, fb)			   \
	p1 = x*_b[0] - y*_b[1];			           \
	p2 = x*_c[0] - y*_c[1];			       	   \
        if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
	rad = fa * box[0] + fb * box[1];   \
	if(min>rad || max<-rad) return 0;

#define AXISTEST_Z0(x, y, fa, fb)			   \
	p0 = x*_a[0] - y*_a[1];				   \
	p1 = x*_b[0] - y*_b[1];			           \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * box[0] + fb * box[1];   \
	if(min>rad || max<-rad) return 0;
	AXISTEST_X01(AB[2], AB[1], fabs(AB[2]), fabs(AB[1]));
	AXISTEST_Y02(AB[2], AB[0], fabs(AB[2]), fabs(AB[0]));
	AXISTEST_Z12(AB[1], AB[0], fabs(AB[1]), fabs(AB[0]));

	AXISTEST_X01(BC[2], BC[1], fabs(BC[2]), fabs(BC[1]));
	AXISTEST_Y02(BC[2], BC[0], fabs(BC[2]), fabs(BC[0]));
	AXISTEST_Z0(BC[1], BC[0], fabs(BC[1]), fabs(BC[0]));

	AXISTEST_X2(AC[2], AC[1], fabs(AC[2]), fabs(AC[1]));
	AXISTEST_Y1(AC[2], AC[0], fabs(AC[2]), fabs(AC[0]));
	AXISTEST_Z12(AC[1], AC[0], fabs(AC[1]), fabs(AC[0]));
#undef AXISTEST_X01
#undef AXISTEST_X2
#undef AXISTEST_Y02
#undef AXISTEST_Y1
#undef AXISTEST_Z12
#undef AXISTEST_Z0

	// Bullets 1
#define FINDMINMAX(x0,x1,x2,min,max) \
	min = max = x0;   \
	if(x1<min) min=x1;\
	if(x1>max) max=x1;\
	if(x2<min) min=x2;\
	if(x2>max) max=x2;
	FINDMINMAX(_a[0],_b[0],_c[0],min,max);
	if(min>box[0] || max<-box[0]) return 0;
	FINDMINMAX(_a[1],_b[1],_c[1],min,max);
	if(min>box[1] || max<-box[1]) return 0;
	FINDMINMAX(_a[2],_b[2],_c[2],min,max);
	if(min>box[2] || max<-box[2]) return 0;
#undef FINDMINMAX
	
	// Bullets 2
	return Object::planeBoxOverlap(n, _a, box);
}

Color Sphere::getTexture(const Vector3& point) const {
	if (mat -> texture) {
		Vector3 vp = (point - center).normalized();
		float PI = 4 * atan(1);
		float phi = acos(-vp.dot(Vector3(0, 0, 1)));
		float theta =  acos(min(max(vp.dot(Vector3(0, 1, 0))/sin(phi), -1.0f), 1.0f));
		float u = phi / PI, v = theta / 2 / PI;
		if (vp.dot(Vector3(1, 0, 0)) < 0 ) v = 1 - v;
		return mat -> texture -> getColor(u, v);
	}
	return Color(1, 1, 1);
}
Color Plane::getTexture(const Vector3& point) const {
	if (mat -> texture) {
		float u = point.dot(UAxis) * mat -> tu;
		float v = point.dot(VAxis) * mat -> tv;
		return mat -> texture -> getColor(u, v);
	}
	return Color(1, 1, 1);
}
Color Triangle::getTexture(const Vector3& point) const {
	return Color(1, 1, 1);
}

vector<Vector3> Sphere::getPos() const {
	vector<Vector3> positions;
	for (int i = 0; i < 50; ++ i) {
		Vector3 p = Vector3(
			(float) (rand() - RAND_MAX / 2), 
			(float) (rand() - RAND_MAX / 2), 
			(float) (rand() - RAND_MAX / 2)
		).normalized();
		positions.push_back(p * radius + center);
	}
	return positions;
}
vector<Vector3> Plane::getPos() const {
	vector<Vector3> positions;
	int parts = 4;
	float uu = ulen / parts;
	float vu = vlen / parts;
	for (int i = 0; i < parts; ++ i)
		for (int j = 0; j < parts; ++ j) {
			float u = ((float)i + (float) rand() / (float) RAND_MAX) * uu;
			float v = ((float)j + (float) rand() / (float) RAND_MAX) * vu;
			positions.push_back(ul + udir * u + vdir * v);
		}
	return positions;
}
vector<Vector3> Triangle::getPos() const {
	vector<Vector3> positions;
	for (int i = 0; i < 40; ++ i) {
		float u = ((float) rand() / (float) RAND_MAX);
		float v = ((float) rand() / (float) RAND_MAX);
		if (u + v < 1)
			positions.push_back(a + (b - a) * u + (c - a) * v);
	}
	return positions;
}

bool Object::planeBoxOverlap(const Vector3& norm, const Vector3& vert, const Vector3& box) {
	float vmin[3],vmax[3],v;
	for(int q = 0; q < 3; q++) {
		v=vert[q];
		if(norm[q] > 0.0f) {
			vmin[q] = -box[q] - v;
			vmax[q] = box[q] - v;
		} else {
			vmin[q]= box[q] - v;
			vmax[q]= -box[q] - v;
		}
	}
	if(norm.dot(Vector3(vmin[0], vmin[1], vmin[2])) > 0.0f) return 0;
	if(norm.dot(Vector3(vmax[0], vmax[1], vmax[2])) >= 0.0f) return 1;
	return 0;
}
