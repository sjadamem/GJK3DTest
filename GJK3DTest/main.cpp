#define _USE_MATH_DEFINES

#include <iostream>
#include <time.h>
#include <vector>
#include <cmath>
#include <map>
#include <list>
#include <array>
#include <memory>
#include <typeinfo>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/vec3.hpp>

struct transform
{
public:
	glm::vec3 mEulers;

	glm::vec3 mPosition;
	glm::quat mRotation;
	glm::vec3 mScale;

	glm::mat4 localMatrix;

	transform(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 eul = glm::vec3(0.0f), glm::vec3 sca = glm::vec3(1.0f)) : 
		mPosition(pos), mRotation(glm::radians(eul)), mScale(sca) 
	{ 
		updateLocalMatrix(); 
	}

	~transform() { }
	
	void translate(glm::vec3 trans) 
	{ 
		mPosition += trans; 
		updateLocalMatrix(); 
	}
	
	void rotate(glm::vec3 rotate) 
	{ 
		mRotation = glm::rotate(mRotation, glm::radians(rotate.x), glm::vec3(1.0f, 0.0f, 0.0f));
		mRotation = glm::rotate(mRotation, glm::radians(rotate.y), glm::vec3(0.0f, 1.0f, 0.0f));
		mRotation = glm::rotate(mRotation, glm::radians(rotate.z), glm::vec3(0.0f, 0.0f, 1.0f));
		updateLocalMatrix(); 
	}
	
	void scale(glm::vec3 scale)
	{
		mScale += scale;
		updateLocalMatrix();
	}

private:

	void updateLocalMatrix()
	{
		glm::mat4 mat(1.0f);

		mat = glm::translate(mat, mPosition);
		mat *= glm::mat4_cast(mRotation);
		mat = glm::scale(mat, mScale);

		localMatrix = mat;
	}
};

struct shape
{
	transform trans;
	std::vector<glm::vec3> points;

	shape(transform t) : trans(t) {}

	~shape() {}

	void translate(glm::vec3& trans) { this->trans.translate(trans); }
	void rotate(glm::vec3 rotate) { this->trans.rotate(rotate); }
	void scale(glm::vec3 scale) { this->trans.scale(scale); }

	glm::vec3 furthestPoint(glm::vec3 dir)
	{
		glm::vec3 furthestPoint;
		float maxDotProd = -FLT_MAX;

		for (unsigned int i = 0; i < points.size(); i++)
		{
			float distance = glm::dot(dir, points[i]);
			if (distance > maxDotProd)
			{
				maxDotProd = distance;
				furthestPoint = points[i];
			}
		}

		return furthestPoint;
	}

	glm::vec3 center() { return trans.mPosition; }
protected:
	virtual void initialize() {}
};

struct shape2D : shape 
{
	shape2D() : shape(transform()) { }
	shape2D(transform t = transform()) : shape(t) {}

	~shape2D() { }
};

struct square : shape2D 
{
	glm::vec3 min, max;

	square(glm::vec2 point = glm::vec2(1.0f, 1.0f), transform t = transform()) : min(glm::vec3(point, 0.f)), max(glm::vec3(point, 0.f)), shape2D(t) { }
	square(glm::vec2 min, glm::vec2 max, transform t = transform()) : min(glm::vec3(min, 0.f)), max(glm::vec3(max, 0.f)), shape2D(t) { }
	~square() { }
};

struct triangle : shape2D
{

};

struct shape3D : shape 
{
	shape3D(transform t = transform()) : shape(t) { }
};

struct box : shape3D
{
	glm::vec3 min, max;
	
	box(glm::vec3 point = glm::vec3(1.0f, 1.0f, 1.0f), transform t = transform()) : min(-point), max(point), shape3D(t) { }
	box(glm::vec3 min, glm::vec3 max, transform t = transform()) : min(min), max(max), shape3D(t) { }

	~box() { }
};

//Pyramid yet to be properly implemented. Is it possible to do Pyramid and Cube collision??
struct pyramid : shape3D
{
};

/*
struct trianglepyramid : pyramid
{
	trianglepyramid(glm::vec3* points, glm::vec3 trans = glm::vec3()) :
		pyramid(points, 5, trans)
	{ }
};

struct squarepyramid : pyramid
{
	squarepyramid(glm::vec3* points, glm::vec3 trans = glm::vec3()) :
		pyramid(points, 6, trans)
	{ }
};
*/

struct sphere : shape3D 
{
	float radius;

	sphere(glm::vec3 center = glm::vec3(0.0f), float r = 1.0f) : radius(r), shape3D(transform(center)) { }
	~sphere() { }

	void translate(glm::vec3 & trans) { this->trans.rotate(trans); }
//	void rotate(glm::vec3& rotate) {}
	void scale(float scale) { radius += scale; }

	glm::vec3& center() { return trans.mPosition; }
};

struct capsule : shape3D
{
	float length, radius;

	capsule(float l = 1.0f, float r = 1.0f, transform t = transform()) : length(l), radius(r), shape3D(t) { }
	~capsule() { }

	void turn(glm::vec3 eulers)
	{
		trans.rotate(eulers);
	}

	glm::vec3 top()
	{
		return glm::vec3(0.0f, length / 2.0f, 0.0f) * glm::mat3(trans.localMatrix);
	}

	glm::vec3 bot()
	{
		return glm::vec3(0.0f, -length / 2.0f, 0.0f) * glm::mat3(trans.localMatrix);
	}
};

struct simplex
{
public:
	simplex() : mSize(0), mPoints({ glm::vec3(), glm::vec3(), glm::vec3(), glm::vec3() }) { }
	~simplex() { }

	simplex& operator=(std::initializer_list<glm::vec3> list)
	{
		for (auto v = list.begin(); v != list.end(); v++)
			mPoints[std::distance(list.begin(), v)] = *v;

		mSize = list.size();
		return *this;
	}

	void push_front(glm::vec3 point)
	{
		mPoints = { point, mPoints[0], mPoints[1], mPoints[2] };
		mSize = std::min(mSize + 1, 4u);
	}

	glm::vec3& operator[] (unsigned int index) { return mPoints[index]; }
	unsigned int size() const { return mSize; }
	auto begin() const { return mPoints.begin(); }
	auto end() const { return mPoints.end() - (4 - mSize); }

private:
	
	std::array<glm::vec3, 4> mPoints;
	unsigned int mSize;
};

glm::vec3 origin(0.0f);

bool GJK3D(shape& shapeA, shape& shapeB);

glm::vec3 supportFunction(shape& shapeA, shape& shapeB, glm::vec3& dir);

bool handleSimplex(simplex& simplex, glm::vec3& dir);

bool lineCase3D(simplex& simplex, glm::vec3& dir);
bool triangleCase3D(simplex& simplex, glm::vec3& dir);
bool tetrahedronCase3D(simplex& simplex, glm::vec3& dir);

bool sameDirection(const glm::vec3& dir, const glm::vec3& ao);

bool BoxBoxCollision		(box& box1,		box& box2);

bool SphereSphereCollision	(sphere& sph1,	sphere& sph2);
bool SphereBoxCollision		(sphere& sph,	box& box);

bool CapsuleCapsuleCollision(capsule& cap1,	capsule& cap2);
bool CapsuleSphereCollision	(capsule& cap,	sphere& sph);
bool CapsuleBoxCollision	(capsule& cap,	box& box);


glm::vec3 closestPointOnLineSegment(glm::vec3& A, glm::vec3& B, glm::vec3& point);

float saturate(float val);

int main()
{
	srand((unsigned int)time(NULL));

	//Triangle
	std::vector<glm::vec3> pointsB = {
		glm::vec3(-0.1f, -0.5f, 0.0f),
		glm::vec3( 0.1f, -0.5f, 0.0f),
		glm::vec3( 0.0f,  0.5f, 0.0f)
	};

//	box* b1 = new box(glm::vec3(1.f));
//	box* b2 = new box(glm::vec3(2.f, 1.f, 1.f));
	sphere* s1 = new sphere(glm::vec3(1.f, 0.f, 0.f));
//	sphere* s2 = new sphere(glm::vec3(3.f, 0.f, 0.0f), 3.f);
	capsule* c1 = new capsule(2.f, 1.f, transform(glm::vec3(0.f, -2.f, 0.f)));
//	capsule* c2 = new capsule(5.f, 2.f, transform(glm::vec3(0.f), glm::vec3(0.f, 0.f, 90.f)));

	//WORKING
//	if (CapsuleCapsuleCollision(*c1, *c2))
//		std::cout << "COLLISION DETECTED::Capsule 1, Capusle 2" << std::endl << std::endl;

	//WORKING
//	if (CapsuleSphereCollision(*c1, *s1))
//		std::cout << "COLLISION DETECTED::Capsule 1, Sphere 1" << std::endl << std::endl;

	//WORKING
//	if (SphereSphereCollision(*s1, *s2))
//		std::cout << "COLLISION DETECTED::Sphere 1, Sphere 2" << std::endl << std::endl;

	//WORKING
//	if (SphereBoxCollision(*s1, *b1))
//		std::cout << "COLLISION DETECTED::Sphere 1, Box 1" << std::endl << std::endl;

	return 0;
}

bool GJK3D(shape& shapeA, shape& shapeB)
{
	simplex simplex;
	glm::vec3 dir;
	glm::vec3 support;
	//Start with a random direction
	dir = glm::normalize(shapeB.center() - shapeA.center());
	//Append support point using the difference of the furthest point of each shape in opposing directions
	support = supportFunction(shapeA, shapeB, dir);
	simplex.push_front(support);

	dir = glm::normalize(-support);

	while (true)
	{
		//Generate new support point using the new direction
		support = supportFunction(shapeA, shapeB, dir);
		//Check if this support point passes through the origin
		if (glm::dot(support, dir) <= 0.0f)
			//If not, then the simplex could not contain the origin and GJK fails
			return false;

		//The support point passes the origin. Append the point to the simplex
		simplex.push_front(support);

		if (handleSimplex(simplex, dir))
			return true;
	}
}

glm::vec3 supportFunction(shape& shapeA, shape& shapeB, glm::vec3& dir)
{
	return shapeA.furthestPoint(dir) - shapeB.furthestPoint(-dir);
}

bool handleSimplex(simplex& simplex, glm::vec3& dir)
{
	switch (simplex.size())
	{
	case 2: return lineCase3D(simplex, dir); 
		break;
	case 3: return triangleCase3D(simplex, dir); 
		break;
	case 4: return tetrahedronCase3D(simplex, dir); 
		break;
	}

	return false;
}

bool lineCase3D(simplex& simplex, glm::vec3& dir)
{
	glm::vec3 A = simplex[0];
	glm::vec3 B = simplex[1];

	glm::vec3 AB = B - A;
	glm::vec3 AO = -A;

	glm::vec3 ABperp = glm::cross(glm::cross(AB, AO), AB);
	dir = glm::vec3(0.0f);

	if (sameDirection(AB, AO))
		dir = glm::normalize(ABperp);
	else
	{
		simplex = { A };
		dir = glm::normalize(AO);
	}

	return false;
}

bool triangleCase3D(simplex& simplex, glm::vec3& dir)
{
	glm::vec3 A = simplex[0];
	glm::vec3 B = simplex[1];
	glm::vec3 C = simplex[2];

	glm::vec3 AB = B - A;
	glm::vec3 AC = C - A;
	glm::vec3 AO = -A;

	glm::vec3 ABC = glm::cross(AB, AC);

	if (sameDirection(glm::cross(ABC, AC), AO))
	{
		if (sameDirection(AC, AO))
		{
			simplex = { A, C };
			dir = glm::normalize(glm::cross(glm::cross(AC, AO), AC));
		}
		else
			return lineCase3D(simplex = { A, B }, dir);
	}
	else
	{
		if (sameDirection(glm::cross(AB, ABC), AO))
			return lineCase3D(simplex = { A, B }, dir);
		else
		{
			if (sameDirection(ABC, AO))
				dir = glm::normalize(ABC);
			else
			{
				simplex = { A, C, B };
				dir = glm::normalize(-ABC);
			}
		}
	}

	return false;
}

bool tetrahedronCase3D(simplex& simplex, glm::vec3& dir)
{
	glm::vec3 A = simplex[0];
	glm::vec3 B = simplex[1];
	glm::vec3 C = simplex[2];
	glm::vec3 D = simplex[3];

	glm::vec3 AB = B - A;
	glm::vec3 AC = C - A;
	glm::vec3 AD = D - A;
	glm::vec3 AO = -A;

	glm::vec3 ABC = glm::cross(AB, AC);
	glm::vec3 ACD = glm::cross(AC, AD);
	glm::vec3 ADB = glm::cross(AD, AB);

	if (sameDirection(ABC, AO))
		return triangleCase3D(simplex = { A, B, C }, dir);
	if (sameDirection(ACD, AO))
		return triangleCase3D(simplex = { A, C, D }, dir);
	if (sameDirection(ADB, AO))
		return triangleCase3D(simplex = { A, D, B }, dir);

	return true;
}

bool sameDirection(const glm::vec3& dir, const glm::vec3& ao)
{
	return glm::dot(dir, ao) > 0.0f;
}

bool SphereSphereCollision(sphere& sphere1, sphere& sphere2)
{
	glm::vec3 distance = sphere2.center() - sphere1.center();

	float length = glm::length(distance);
	float radii = (sphere1.radius + sphere2.radius);

	return (length < radii) ? true : false;
}

bool SphereBoxCollision(sphere& sphere, box& box)
{
	float sq = 0;

	auto a = [&](float pn, float bmin, float bmax) -> float{
		float out = 0;
		float v = pn;

		if (v < bmin)
		{
			float val = bmin - v;
			out += val * val;
		}
		if (v > bmax)
		{
			float val = bmax - v;
			out += val * val;
		}

		return out;
	};

	sq += a(sphere.center().x, box.min.x, box.max.x);
	sq += a(sphere.center().y, box.min.y, box.max.y);
	sq += a(sphere.center().z, box.min.z, box.max.z);

	return sq <= (sphere.radius * sphere.radius);
}

bool CapsuleCapsuleCollision(capsule& capA, capsule& capB)
{
	glm::vec3 aTop = capA.top();
	glm::vec3 aBot = capA.bot();
	glm::vec3 bTop = capB.top();
	glm::vec3 bBot = capB.bot();

	glm::vec3 v0 = bBot - aBot;
	glm::vec3 v1 = bTop - aBot;
	glm::vec3 v2 = bBot - aTop;
	glm::vec3 v3 = bTop - aTop;

	float d0 = glm::length2(v0);
	float d1 = glm::length2(v1);
	float d2 = glm::length2(v2);
	float d3 = glm::length2(v3);

	glm::vec3 bestA;
	if (d2 < d0 || d2 < d1 || d3 < d0 || d3 < d1)
		bestA = aTop;
	else
		bestA = aBot;

	glm::vec3 bestB = closestPointOnLineSegment(bBot, bTop, bestA);
	bestA = closestPointOnLineSegment(aBot, aTop, bestB);

	sphere* s1 = new sphere(bestA, capA.radius);
	sphere* s2 = new sphere(bestB, capB.radius);

	return SphereSphereCollision(*s1, *s2);
}

bool CapsuleSphereCollision(capsule& cap, sphere& sph)
{
	glm::vec3 center= sph.center();
	glm::vec3 bot	= cap.bot();
	glm::vec3 top	= cap.top();

	glm::vec3 pointOnCap = closestPointOnLineSegment(bot, top, center);

	sphere* s = new sphere(pointOnCap, cap.radius);

	return SphereSphereCollision(sph, *s);
}

glm::vec3 closestPointOnLineSegment(glm::vec3& A, glm::vec3& B, glm::vec3& point)
{
	glm::vec3 AB = B - A;
	float t = glm::dot(point - A, AB) / glm::length2(AB);

	return A + saturate(t) * AB;
}

float saturate(float val)
{
	return glm::min(glm::max(val, 0.0f), 1.0f);
}