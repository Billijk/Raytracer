#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "material.h"
#include "vector3.h"
#include "color.h"
#include <string>
#include <vector>

extern const float EPS;

class Object {
protected:
	bool light;
	Material* mat;
public:
	virtual ~Object() {}
	std::string getType() const { return light ? "Light" : "Others"; }
	virtual Vector3 getNormVec(const Vector3&) const = 0;
	virtual Vector3 collideWith(const Vector3&, const Vector3&) const = 0; 
	virtual float calcExtreme(int, bool) const = 0;
	virtual bool intersectWithBox(float, float, float, float, float, float) const = 0;
	virtual Color getTexture(const Vector3&) const = 0;
	virtual std::vector<Vector3> getPos() const = 0;
	void setMaterial(Material* _mat) { mat = _mat; }
	Material* getMaterial() { return mat; }
	Color getColor() { return mat -> color; }

	// 检测平面与axis-aligned box的overlap（盒子以原点为中心）
	static bool planeBoxOverlap(const Vector3&, const Vector3&, const Vector3&);
};

class Sphere: public Object {
	Vector3 center;
	float radius;
public:
	Sphere(Vector3, float, bool l = false);
	~Sphere() {}
	virtual Vector3 getNormVec(const Vector3&) const;
	virtual Vector3 collideWith(const Vector3&, const Vector3&) const;
	virtual float calcExtreme(int, bool) const;
	virtual bool intersectWithBox(float, float, float, float, float, float) const;
	virtual std::vector<Vector3> getPos() const;
	virtual Color getTexture(const Vector3&) const;
};

class Plane: public Object {
// Square actually =_=
	Vector3 ul, dr;			// up-left && down-right
	Vector3 n;				// normal vector
	Vector3 udir, vdir;		// 2 dir vectors
	float ulen, vlen;		// 2 lengths
	Vector3 UAxis, VAxis;	// for texture
public:
	Plane(Vector3, Vector3, Vector3, bool l = false);
	~Plane() {}
	virtual Vector3 getNormVec(const Vector3&) const;
	virtual Vector3 collideWith(const Vector3&, const Vector3&) const;
	virtual float calcExtreme(int, bool) const;
	virtual bool intersectWithBox(float, float, float, float, float, float) const;
	virtual std::vector<Vector3> getPos() const;
	virtual Color getTexture(const Vector3&) const;
};

class Triangle: public Object {
	Vector3 a, b, c;		// 3 vertices
	Vector3 na, nb, nc, n; 	// norms
public:
	Triangle(Vector3, Vector3, Vector3, bool l = false);
	Triangle(Vector3, Vector3, Vector3, Vector3, Vector3, Vector3, bool l = false);
	~Triangle() {}
	virtual Vector3 getNormVec(const Vector3&) const;
	virtual float calcExtreme(int, bool) const;
	virtual bool intersectWithBox(float, float, float, float, float, float) const;
	virtual Vector3 collideWith(const Vector3&, const Vector3&) const;
	virtual std::vector<Vector3> getPos() const;
	virtual Color getTexture(const Vector3&) const;
};

#endif
