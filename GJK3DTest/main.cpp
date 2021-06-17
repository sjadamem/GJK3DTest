#define _USE_MATH_DEFINES

#include <iostream>
#include <time.h>
#include <vector>
#include <cmath>
#include <map>
#include <list>
#include <array>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/vec3.hpp>

struct transform
{
public:
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;

	transform() : position(glm::vec3()), rotation(glm::quat()), scale(glm::vec3()) {}
	transform(glm::vec3 pos, glm::quat rot, glm::vec3 sca) : position(pos), rotation(rot), scale(sca) {}
	transform(glm::vec3 pos, glm::vec3 eul, glm::vec3 sca) : position(pos), rotation(eul), scale(sca) {}
	transform(float xPos, float yPos, float zPos, float xRot, float yRot, float zRot, float xSca, float ySca, float zSca) :
		position(glm::vec3(xPos, yPos, zPos)), rotation(glm::vec3(xRot, yRot, zRot)), scale(glm::vec3(xSca, ySca, zSca)) 
	{}
};

struct shape
{
	glm::vec3* mPoints;
	glm::vec3 mCenter;
	unsigned int mSize;

	shape() {};
	shape(glm::vec3* points, unsigned int size) : mSize(size)
	{
		mPoints = std::move(points);
	}

	~shape()
	{
		delete[] mPoints;
	}

	void translate(glm::vec3& trans)
	{
		for (unsigned int i = 0; i < mSize; i++)
			mPoints[i] += trans;

		mCenter += trans;
	}

	glm::vec3 furthestPoint(glm::vec3 dir)
	{
		glm::vec3 furthestPoint;
		float maxDotProd = -FLT_MAX;

		for (unsigned int i = 0; i < mSize; i++)
		{
			float distance = glm::dot(dir, mPoints[i]);
			if (distance > maxDotProd)
			{
				maxDotProd = distance;
				furthestPoint = mPoints[i];
			}
		}

		return furthestPoint;
	}

protected:
	virtual void centerPoint() {}
};

struct cube : shape
{
	cube(glm::vec3* points, glm::vec3 trans = glm::vec3()) :
		shape(points, 8)
	{
		centerPoint();
		translate(trans);
	}

protected:
	void centerPoint()
	{
		float pX, pY, pZ, nX, nY, nZ;
		pX = pY = pZ = nX = nY = nZ = 0.0f;

		for (unsigned int i = 0; i < mSize; i++)
		{
			glm::vec3 p = mPoints[i];

			if (p.x > pX) 
				pX = p.x;
			else if (p.x < nX) 
				nX = p.x;
			
			if (p.y > pY) 
				pY = p.y;
			else if (p.y < nY) 
				nY = p.y;
			
			if (p.z > pZ) 
				pZ = p.z;
			else if (p.z < nZ) 
				nZ = p.z;
		}

		mCenter = glm::vec3(pX + nX, pY + nY, pZ + nZ);
	}
};

//Pyramid yet to be properly implemented. Is it possible to do Pyramid and Cube collision??
struct pyramid : shape
{
	pyramid(glm::vec3* points, unsigned int size, glm::vec3 trans = glm::vec3()) :
		shape(points, size)
	{
		centerPoint();
		translate(trans);
	}

protected:
	void centerPoint()
	{
		float y = (mPoints[mSize - 1].y - mPoints[0].y) * (3.0f / 4.0f);
		mCenter = mPoints[mSize - 1] - glm::vec3(0, y, 0);
	}
};

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

struct sphere 
{
	glm::vec3 origin;
	float radius;

	sphere() : origin(glm::vec3()), radius(1.0f) { }
	sphere(glm::vec3 o, float r) : origin(o), radius(r) { }
	~sphere() {}

	void translate(glm::vec3 & trans) { origin += trans; }
	void rotate(glm::vec3& rotate) {}
	void scale(float scale) { radius += scale; }
};

struct capsule : sphere
{
	float length;
	glm::quat rotate;

	capsule() : length(1.0f), rotate(glm::quat()), sphere() {}
	capsule(float l = 1.0f, float r = 1.0f, glm::vec3 o = glm::vec3(), glm::vec3 eul = glm::vec3()) :
		length(l), rotate(glm::quat(eul)), sphere(o, r) {}
	~capsule() {}

	glm::vec3 top()
	{
		return origin + ((rotate * glm::vec3(1.0f, 0.0f, 0.0f)) * length / 2.0f);
	}

	glm::vec3 bot()
	{
		return origin - ((rotate * glm::vec3(1.0f, 0.0f, 0.0f)) * length / 2.0f);
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

bool sphereCollision(sphere& sphere1, sphere& sphere2);

bool capsuleCollision(capsule& cap1, capsule& cap2);
glm::vec3 closestPointOnLineSegment(glm::vec3& A, glm::vec3& B, glm::vec3& point);

float saturate(float val);

int main()
{
	srand((unsigned int)time(NULL));
	
	//Square
	std::vector<glm::vec3> pointsA = {
		glm::vec3(-1.0f,-1.0f, 0.0f),
		glm::vec3( 1.0f,-1.0f, 0.0f),
		glm::vec3( 1.0f, 1.0f, 0.0f),
		glm::vec3(-1.0f, 1.0f, 0.0f)
	};

	//Triangle
	std::vector<glm::vec3> pointsB = {
		glm::vec3(-0.1f, -0.5f, 0.0f),
		glm::vec3( 0.1f, -0.5f, 0.0f),
		glm::vec3( 0.0f,  0.5f, 0.0f)
	};
	
	//Cube
	glm::vec3 pointsC[] = {
		glm::vec3(-1.0f,-1.0f, 1.0f),
		glm::vec3( 1.0f,-1.0f, 1.0f),
		glm::vec3( 1.0f, 1.0f, 1.0f),
		glm::vec3(-1.0f, 1.0f, 1.0f),

		glm::vec3(-1.0f,-1.0f,-1.0f),
		glm::vec3( 1.0f,-1.0f,-1.0f),
		glm::vec3( 1.0f, 1.0f,-1.0f),
		glm::vec3(-1.0f, 1.0f,-1.0f)
	};

	glm::vec3 pointsD[] = {
		glm::vec3(-0.25f,-0.25f, 0.25f),
		glm::vec3( 0.25f,-0.25f, 0.25f),
		glm::vec3( 0.25f, 0.25f, 0.25f),
		glm::vec3(-0.25f, 0.25f, 0.25f),
							  
		glm::vec3(-0.25f,-0.25f,-0.25f),
		glm::vec3( 0.25f,-0.25f,-0.25f),
		glm::vec3( 0.25f, 0.25f,-0.25f),
		glm::vec3(-0.25f, 0.25f,-0.25f)
	};

	cube* cube1 = new cube(pointsC);
	cube* cube2 = new cube(pointsD, glm::vec3(0.0f, 1.0f, 3.0f)); //glm::vec3(1.0f, 0.25f, 0.0f)
//	pyramid* sPyramid = new squarepyramid(pointsE);
//	pyramid* tPyramid = new trianglepyramid(pointsF, glm::vec3(0.0f, -2.0f, 0.0f));

	if (GJK3D(*cube1, *cube2))
		std::cout << "Collision detected!" << std::endl;
	
	int counter = 0;
	glm::vec3 change(0.01f, 0.0f, 0.0f);

/*
	while (cube2->mCenter.x >= -2.5f)
	{
		if (cube2->mCenter.x >= 2.5f)
			change = -change;

		if (GJK3D(*cube1, *cube2))
		{
			std::cout << "Collision detected!!" << std::endl << std::endl;
			
			std::cout << "Cube 1 Center\n" << cube1->mCenter.x << "\t" << cube1->mCenter.y << std::endl;
			for (unsigned int i = 0; i < cube1->mSize; i++)
				std::cout << cube1->mPoints[i].x << "::" << cube1->mPoints[i].y << "\t";

			std::cout << "\nCube 2 Center\n" << cube2->mCenter.x << "\t" << cube2->mCenter.y << std::endl;
			for (unsigned int i = 0; i < cube2->mSize; i++)
				std::cout << cube2->mPoints[i].x << "::" << cube2->mPoints[i].y << "\t";
		}

		cube2->translate(change);
	}
	*/

//	cout << (char)('A' + 1) << endl;

	sphere* sphere1 = new sphere();
	sphere* sphere2 = new sphere(glm::vec3(5.0f, 3.0f, 1.0f), 2);
	float scale = 0.01f;
	while (sphere2->radius <= 5.0f)
	{
		if (sphere2->origin.x >= 5.0f)
			change = -change;

		if (sphereCollision(*sphere1, *sphere2))
		{
			std::cout << "Collision detected!" << std::endl;
			std::cout << "SPHERE 1\nX::" << sphere1->origin.x << "\tY::" << sphere1->origin.y << "\tZ::" << sphere1->origin.z << "\nRADIUS\t= " << sphere1->radius << std::endl;
			std::cout << "SPHERE 2\nX::" << sphere2->origin.x << "\tY::" << sphere2->origin.y << "\tZ::" << sphere2->origin.z << "\nRADIUS\t= " << sphere2->radius << std::endl << std::endl;
		}

		sphere2->scale(scale);
	}

	glm::quat rot(glm::vec3(0.0f, 0.0f, glm::radians(135.0f)));
	std::cout << "W::" << rot.w << "\tX::" << rot.x << "\tY::" << rot.y << "\tZ::" << rot.z << std::endl;
	
	glm::vec3 eul = glm::eulerAngles(rot);
	std::cout << "X::" << eul.x << "\tY::" << eul.y << "\tZ::" << eul.z << std::endl;
	
	glm::vec3 dir = rot * glm::vec3(1.0f, 0.0f, 0.0f);
	std::cout << "X::" << dir.x << "\tY::" << dir.y << "\tZ::" << dir.z << std::endl;
	
	glm::vec3 point = dir * 4.0f;
	std::cout << "X::" << point.x << "\tY::" << point.y << "\tZ::" << point.z << std::endl;

	capsule* cap1 = new capsule(3.0f, 0.5f, glm::vec3(), glm::vec3(0.0f, 0.0f, glm::radians(90.0f)));
	capsule* cap2 = new capsule(2.0f, 0.5f, glm::vec3(1.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, glm::radians(135.0f)));
	glm::vec3 aTop = cap1->top();
	glm::vec3 aBot = cap1->bot();
	glm::vec3 bTop = cap2->top();
	glm::vec3 bBot = cap2->bot();

	if (capsuleCollision(*cap1, *cap2))
		std::cout << "Capsule collision detected!" << std::endl;

	return 0;
}

bool GJK3D(shape& shapeA, shape& shapeB)
{
	simplex simplex;
	glm::vec3 dir;
	glm::vec3 support;
	//Start with a random direction
	dir = glm::normalize(shapeB.mCenter - shapeA.mCenter);
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

	if (sameDirection(AB, AO) > 0.0f)
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

bool sphereCollision(sphere& sphere1, sphere& sphere2)
{
	glm::vec3 distance = sphere2.origin - sphere1.origin;

	float length = glm::length(distance);
	float radii = (sphere1.radius + sphere2.radius);

	return (length < radii) ? true : false;
}

bool capsuleCollision(capsule& capA, capsule& capB)
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

	return sphereCollision(*s1, *s2);
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