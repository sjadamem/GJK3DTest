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

	float a = 0.0f;
	float &b = a;
	b = 1.0f;
	std::cout << "A::" << a << "\tB::" << b << std::endl;

	if (GJK3D(*cube1, *cube2))
		std::cout << "Collision detected!" << std::endl;
	
	int counter = 0;
	glm::vec3 change(0.01f, 0.0f, 0.0f);

	
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

//	cout << (char)('A' + 1) << endl;

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