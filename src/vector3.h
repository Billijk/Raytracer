#ifndef _VECTOR_3_
#define _VECTOR_3_

extern const float EPS;

class Vector3 {
public:
	float x, y, z;
	Vector3() { x = y = z = 0; }
	Vector3(float, float, float);
	Vector3(const Vector3&);
	float operator [] (const int&) const;
	friend Vector3 operator + (const Vector3&, const Vector3&);
	friend Vector3 operator - (const Vector3&, const Vector3&);
	friend Vector3 operator * (const Vector3&, const float&);
	friend Vector3 operator / (const Vector3&, const float&);
	friend Vector3 operator += (Vector3&, const float&);
	friend Vector3 operator -= (Vector3&, const float&);
	friend Vector3 operator *= (Vector3&, const float&);
	friend Vector3 operator /= (Vector3&, const float&);
	void normalize();
	Vector3 normalized();
	bool isZeroVector() const;
	float norm() const;
	float distance(const Vector3&) const;
	float distToLine(const Vector3&, const Vector3&) const;
	float dot(const Vector3&) const;
	Vector3 cross(const Vector3&) const;
};

#endif
