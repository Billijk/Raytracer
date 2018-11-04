#include "ray.h"
#include "color.h"
#include "scene.h"
#include "vector3.h"
#include "object.h"
#include "material.h"
#include <cmath>
#include <vector>
#include <map>
#include <cstdio>
#include <algorithm>

using namespace std;

Ray::Ray(Vector3 s, Vector3 d, bool inside) {
	startPoint = s, direction = d, insideObj = inside;
	if (!d.isZeroVector()) direction.normalize();
}

pair<Object*, Vector3> Ray::detectCollide(Scene* s) {
	return s -> kdtree -> traverse(s -> kdtree -> getRoot(), startPoint, direction);
}

Color Ray::calcReflection(Scene* scene, Object* obj, Vector3 point, int depth, bool pt) {
	Color alpha = obj -> getMaterial() -> color * obj -> getTexture(point);
	return reflectRay(point, obj -> getNormVec(point)).rayTracing(scene, depth + 1, pt) * alpha;
}

Color Ray::calcRefraction(Scene* scene, Object* obj, Vector3 point, int depth, bool pt) {
	float n = obj -> getMaterial() -> r_index;
	if (insideObj) n = 1.0 / n;
	Ray ray = refractRay(point, obj -> getNormVec(point) * (insideObj ? -1 : 1), n);

	if (ray.getDirection().isZeroVector()) return Color(0, 0, 0);	// full reflection
	else {
		Color color = ray.rayTracing(scene, depth + 1, pt);
		color *= (obj -> getMaterial() -> absorb * -point.distance(this -> startPoint)).Exp();
		return color;
	}
}

Color Ray::calcDiffusion(Scene* scene, Object* light, Object* obj, Vector3 point) {
	float color = 0;
	auto positions = light -> getPos();
	Material* mat = obj -> getMaterial();
	Vector3 N = obj -> getNormVec(point);
	for (const auto& pos: positions) {
		Ray ray(point, pos - point);
		auto hit = ray.detectCollide(scene);
		if (hit.first == light) {
			Vector3 R = (pos - point).normalized();
			if (mat -> diff > EPS) color += (mat -> diff * fabs(R.dot(N)));
			if (mat -> spec > EPS) color += (mat -> spec * pow(R.dot(reflectRay(point, N).getDirection()), 50));
		}
	}
	color /= positions.size();
	return mat -> color * obj -> getTexture(point) * color * light -> getColor();
}

Ray Ray::reflectRay(Vector3 point, Vector3 N) {
	return Ray(point, direction - N * 2 * direction.dot(N));
}

Ray Ray::refractRay(Vector3 point, Vector3 N, float rindex) {
	float cos1 = -N.dot(direction);
	float cos22 = 1 - (1 - cos1 * cos1) / (rindex * rindex);
	if (cos22 > 0.0f) {
		Vector3 T = direction / rindex + N * (cos1 / rindex - sqrt(cos22));
		return Ray(point, T, !insideObj);
	} else {
		return Ray(point, Vector3(0, 0, 0));
	}
}

Color Ray::rayTracing(Scene* scene, int depth, bool pt) {
	Color c(0, 0, 0);
	if (depth > 8) return c;
	auto hit = detectCollide(scene);
	if (hit.first == NULL) return c;

	Material* mat = hit.first -> getMaterial();
	
	if (hit.first -> getType() == "Light")
		return mat -> color * hit.first -> getTexture(hit.second);

	if (pt) {
		// path tracing
		float rnf = (float) rand() / (float) RAND_MAX;
		float p = mat -> refl;
		if (p > rnf) c = calcReflection(scene, hit.first, hit.second, depth, pt);
		else if ((p += mat -> refr) > rnf)
			c = calcRefraction(scene, hit.first, hit.second, depth, pt);
		else if ((p += (mat -> diff + mat -> spec)) > rnf) {
			Vector3 newd;
			Vector3 norm = hit.first -> getNormVec(hit.second);
			while (newd.isZeroVector() || newd.dot(norm) < 0)
				newd = Vector3(
					(float) (rand() - RAND_MAX / 2), 
					(float) (rand() - RAND_MAX / 2), 
					(float) (rand() - RAND_MAX / 2)
				).normalized();
			c = mat -> color * hit.first -> getTexture(hit.second) * 
				Ray(hit.second, newd).rayTracing(scene, depth + 1, pt);
		}
	} else {
		// naive ray tracing
		if (mat -> refl > EPS) c += calcReflection(scene, hit.first, hit.second, depth, pt) * mat -> refl;
		if (mat -> refr > EPS) c += calcRefraction(scene, hit.first, hit.second, depth, pt) * mat -> refr;
		for (const auto& light: scene -> lights) {
			c += calcDiffusion(scene, light, hit.first, hit.second);
		}
	}

	return c;
}
