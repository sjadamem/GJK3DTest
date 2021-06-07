#define _USE_MATH_DEFINES

#include <iostream>
#include <time.h>
#include <vector>
#include <cmath>
#include <map>
#include <list>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/vec3.hpp>

using namespace std;
/*
struct vec3
{
public:
	float x, y, z;

	vec3() : x(0), y(0), z(0) {};
	vec3(float x, float y, float z) : x(x), y(y), z(z) {};
	vec3(float val) : x(val), y(val), z(val) {}

	friend ostream &operator<<(ostream &output, const vec3 &v) {
		output << "X : " << v.x << "|Y : " << v.y << "|Z : " << v.z;
		return output;
	}
	
	vec3 operator+ (const vec3& val)
	{
		return add(val);
	}

	vec3& operator+= (const vec3& val)
	{
		this->x += val.x;
		this->y += val.y;

		return *this;
	}

	vec3 operator- (const vec3& val)
	{
		return sub(val);
	}

	vec3 operator- ()
	{
		return vec3(-x, -y, -z);
	}
	
	vec3 operator-= (const vec3& val)
	{
		return sub(val);
	}

	vec3 operator* (const float& val)
	{
		return mul(val);
	}

	vec3 operator/ (const float& val)
	{
		return div(val);
	}


	vec3 operator= (vec3 val)
	{
		return val;
	}

	nullptr_t operator= (nullptr_t val)
	{
		return val;
	}

	bool operator== (vec3& val)
	{
		if (this->x == val.x && this->y == val.y && this->z == val.z)
			return true;

		return false;
	}

	static float dot(vec3 a, vec3 b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	static vec3 cross(vec3 a, vec3 b)
	{
		return vec3(
			a.y * b.z - a.z * b.y, 
			a.z * b.x - a.x * b.z, 
			a.x * b.y - a.y * b.x
		);
	}

	static float magnitude(vec3& val)
	{
		return sqrt(val.x * val.x + val.y * val.y + val.z * val.z);
	}

	static vec3 normalize(vec3 val)
	{
		return val / magnitude(val);
	}

private:
	
	vec3 add(vec3 val)
	{
		return vec3(this->x + val.x, this->y + val.y, this->z + val.z);
	}

	vec3 sub(vec3 val)
	{
		return vec3(this->x - val.x, this->y - val.y, this->z - val.z);
	}

	template <typename T>
	vec3 mul(T val)
	{
		return vec3(this->x * val, this->y * val, this->z * val);
	}

	template <typename T>
	vec3 div(T val)
	{
		return vec3(x / val, y / val, z / val);
	}
};
*/

struct shape
{
	glm::vec3 mCenter;

	glm::vec3* mPoints;
	unsigned int mSize;

	shape(glm::vec3* points, unsigned int size, glm::vec3 trans = glm::vec3(0.0f)) 
		: mSize(size), mCenter(glm::vec3(0.0f))
	{ 
		mPoints = move(points); 
		translate(trans);
	}

	void translate(glm::vec3& trans)
	{
		for (unsigned int i = 0; i < mSize; i++)
			mPoints[i] += trans;

		mCenter += trans;
	}

	glm::vec3 furthestPoint(glm::vec3 dir)
	{
		float maxDotProd = glm::dot(dir, mPoints[0]);
		glm::vec3 furthestPoint = mPoints[0];

		for (unsigned int i = 1; i < mSize; i++)
		{
			float val = glm::dot(dir, mPoints[i]);
			if (val > maxDotProd)
			{
				furthestPoint = mPoints[i];
				maxDotProd = val;
			}
		}

		return furthestPoint;
	}
};

struct simplex
{
public:
	simplex() : mSize(3), mLength(0)
	{ 
		mPoints = new glm::vec3[mSize];
	}

	simplex(unsigned int size) : mSize(size), mLength(0)
	{
		mPoints = new glm::vec3[mSize];
	}

	glm::vec3 operator[] (unsigned int index)
	{
		return mPoints[index];
	}

	void append(glm::vec3 val)
	{
		if (mLength == mSize)
			return;

		mPoints[mLength] = val;
		mLength++;
	}

	void remove(glm::vec3 val)
	{
		for (unsigned int i = 0; i < mLength; i++)
		{
			if (mPoints[i] == val)
			{
				if (i < mLength - 1)
					move(mPoints + i + 1, mPoints + mLength, mPoints + i);

				mLength--;
				break;
			}
		}
	}

	unsigned int length()
	{
		return mLength;
	}

private:
	
	glm::vec3* mPoints;
	unsigned int mSize;
	unsigned int mLength;
};

glm::vec3 origin(0.0f);

bool GJK2D(shape& shapeA, shape& shapeB);
bool GJK3D(shape& shapeA, shape& shapeB);

glm::vec3 supportFunction(shape& shapeA, shape& shapeB, glm::vec3& dir);

bool handleSimplex(simplex& simplex, glm::vec3* dir);

bool lineCase2D(simplex& simplex, glm::vec3* dir);
bool triangleCase2D(simplex& simplex, glm::vec3* dir);

bool lineCase3D(simplex& simplex, glm::vec3* dir);
bool triangleCase3D(simplex& simplex, glm::vec3* dir);
bool tetrahedonCase(simplex& simplex, glm::vec3* dir);

bool pointOnTriangle(shape& shape, glm::vec3& point);

bool GJK2D(shape& shapeA, shape& shapeB)
{
	simplex simplex;
	glm::vec3 dir;

	//Start with a random direction
	dir = glm::normalize(shapeB.mCenter - shapeA.mCenter);
	simplex.append(supportFunction(shapeA, shapeB, dir));

	dir = -simplex[0];

	while (true)
	{
		glm::vec3 vec = supportFunction(shapeA, shapeB, dir);
		if (glm::dot(vec, dir) < 0.0f)
			return false;

		simplex.append(vec);
		
		if (handleSimplex(simplex, &dir))
			return true;
	}
}

bool GJK3D(shape& shapeA, shape& shapeB)
{
	simplex simplex;
	glm::vec3 dir;

	//Start with a random direction
	dir = glm::normalize(shapeB.mCenter - shapeA.mCenter);
	//Append support point using the difference of the furthest point of each shape in opposing directions
	simplex.append(supportFunction(shapeA, shapeB, dir));

	dir = glm::normalize(-simplex[0]);

	while (true)
	{
		//Generate new support point using the new direction
		glm::vec3 vec = supportFunction(shapeA, shapeB, dir);
		//Check if this support point passes through the origin
		if (glm::dot(vec, dir) < 0.0f)
			//If not, then the simplex could not contain the origin and GJK fails
			return false;

		//The support point passes the origin. Append the point to the simplex
		simplex.append(vec);

		if (handleSimplex(simplex, &dir))
			return true;
	}
}

glm::vec3 supportFunction(shape& shapeA, shape& shapeB, glm::vec3& dir)
{
	return shapeA.furthestPoint(dir) - shapeB.furthestPoint(-dir);
}

bool handleSimplex(simplex& simplex, glm::vec3* dir)
{
/*
	switch (simplex.length())
	{
	case 2: break;
	case 3: break;
	default: break;
	}
	*/

	unsigned int length = simplex.length();

	if (length == 2)
		return lineCase2D(simplex, dir);
	else if (length == 3)
		return triangleCase3D(simplex, dir);
	
	return triangleCase2D(simplex, dir);
}

bool lineCase2D(simplex& simplex, glm::vec3* dir)
{
	glm::vec3* A = &simplex[0];
	glm::vec3* B = &simplex[1];

	glm::vec3 AB = *B - *A;
	glm::vec3 AO = glm::vec3(0.0f) - *A;

	*dir = glm::normalize(glm::cross(glm::cross(AB, AO), AB));

	float slope = (B->y - A->y) / (B->x - A->x);
	if (slope * B->x + B->y == 0.0f)
		return true;

	return false;
}

bool triangleCase2D(simplex& simplex, glm::vec3* dir)
{
	glm::vec3* A = &simplex[0];
	glm::vec3* B = &simplex[1];
	glm::vec3* C = &simplex[2];

	glm::vec3 AB = *B - *A;
	glm::vec3 AC = *C - *A;
	glm::vec3 AO = origin - *A;

	glm::vec3 ABperp = glm::normalize(glm::cross(glm::cross(AC, AB), AB));
	glm::vec3 ACperp = glm::normalize(glm::cross(glm::cross(AB, AC), AC));

	if (glm::dot(ABperp, AO) > 0.0f)
	{
		simplex.remove(*C);
		*dir = ABperp;
		return false;
	}
	else if (glm::dot(ACperp, AO) > 0.0f)
	{
		simplex.remove(*B);
		*dir = ACperp;
		return false;
	}

	return true;
}


bool triangleCase3D(simplex& simplex, glm::vec3* dir)
{
	glm::vec3* A = &simplex[0];
	glm::vec3* B = &simplex[1];
	glm::vec3* C = &simplex[2];

	glm::vec3 AB = *B - *A;
	glm::vec3 AC = *C - *A;
	glm::vec3 AO = glm::vec3(0.0f) - *A;

	glm::vec3 ABperp = glm::normalize(glm::cross(glm::cross(AC, AB), AB));
	glm::vec3 ACperp = glm::normalize(glm::cross(glm::cross(AB, AC), AC));

	float area = glm::dot(AB, AC) / 2.0f;

	glm::vec3 PA = origin - *A;
	glm::vec3 PB = origin - *B;
	glm::vec3 PC = origin - *C;

	float aArea = glm::dot(PB, PC) / (2.0f * area);
	if (aArea < 0.0f || aArea > 1.0f)
		return false;

	float bArea = glm::dot(PC, PA) / (2.0f * area);
	if (bArea < 0.0f || bArea > 1.0f)
		return false;

	float yArea = 1 - aArea - bArea;
	if (yArea < 0.0f || yArea > 1.0f)
		return false;

	*dir = glm::cross(AB, AC);
	
	return false;
}

bool tetrahedonCase(simplex& simplex, glm::vec3* dir)
{
	return false;
}

bool pointOnTriangle(shape& shape, glm::vec3& point)
{
	glm::vec3* A = &shape.mPoints[0];
	glm::vec3* B = &shape.mPoints[1];
	glm::vec3* C = &shape.mPoints[2];

	glm::vec3 AB = *A - *B;
	glm::vec3 AC = *A - *C;
						 
	float area = glm::length2(glm::cross(AB, AC)) / 2.0f;

	glm::vec3 PA = *A - point;
	glm::vec3 PB = *B - point;
	glm::vec3 PC = *C - point;

	float aArea = glm::length2(glm::cross(PB, PC)) / (2.0f * area);
	if (aArea < 0.0f || aArea > 1.0f)
		return false;

	float bArea = glm::length2(glm::cross(PC, PA)) / (2.0f * area);
	if (bArea < 0.0f || bArea > 1.0f)
		return false;

	float yArea = glm::length2(glm::cross(PA, PB)) / (2.0f * area);
	if (yArea < 0.0f || yArea > 1.0f)
		return false;

	float x = (aArea + bArea + yArea);
	cout << x << endl;
	return (x > 0.999f && x <= 1.0f) ? true : false;
}

int main()
{
	srand((unsigned int)time(NULL));
	
	//Square
	vector<glm::vec3> pointsA = {
		glm::vec3(-1.0f,-1.0f, 0.0f),
		glm::vec3( 1.0f,-1.0f, 0.0f),
		glm::vec3( 1.0f, 1.0f, 0.0f),
		glm::vec3(-1.0f, 1.0f, 0.0f)
	};

	//Triangle
	vector<glm::vec3> pointsB = {
		glm::vec3(-0.1f, -0.5f, 0.0f),
		glm::vec3( 0.1f, -0.5f, 0.0f),
		glm::vec3( 0.0f,  0.5f, 0.0f)
	};
	
	//Cube
	vector<glm::vec3> pointsC = {
		glm::vec3(-1.0f,-1.0f, 1.0f),
		glm::vec3( 1.0f,-1.0f, 1.0f),
		glm::vec3 (1.0f, 1.0f, 1.0f),
		glm::vec3(-1.0f, 1.0f, 1.0f),

		glm::vec3(-1.0f,-1.0f,-1.0f),
		glm::vec3( 1.0f,-1.0f,-1.0f),
		glm::vec3( 1.0f, 1.0f,-1.0f),
		glm::vec3(-1.0f, 1.0f,-1.0f)
	};

	//Pyramid
	vector<glm::vec3> pointsD = {
		glm::vec3(-0.1f,-0.5f, 1.0f),
		glm::vec3( 0.0f, 0.5f, 1.0f),
		glm::vec3(-0.1f,-0.5f,-1.0f),
		glm::vec3( 0.0f, 0.5f,-1.0f),

		glm::vec3( 0.1f,-0.5f, 0.0f)
	};

	vector<glm::vec3> pointsE = {
		glm::vec3( 2.0f, 0.0f, 0.0f),
		glm::vec3( 1.0f, 1.0f,-1.0f),
		glm::vec3(-1.0f,-1.0f, 2.0f)
	};

	shape square(pointsA.data(), pointsA.size());
	shape triangle(pointsB.data(), pointsB.size(), glm::vec3(1.0f, 0.1f, 0.0f));
	shape triangle3D(pointsE.data(), pointsE.size());

	shape cube(pointsC.data(), pointsC.size());
	shape pyramid(pointsD.data(), pointsD.size(), glm::vec3(1.0f, 0.1f, 0.0f));

	glm::vec3 point(0.5f, -0.9f, 0.0f);

	while (point.y < -0.8f)
	{
		if (pointOnTriangle(triangle3D, point))
			cout << "Collision detected\nPoint\t" << point.x << "::" << point.y << "::" << point.z << endl;

		point += glm::vec3(0.0f, 0.0001f, 0.0f);
	}

	unsigned int counter = 0;
	glm::vec3 change(0.01, 0.0f, 0.0f);

	glm::vec3 data[] = { glm::vec3(), glm::vec3(1.0f), glm::vec3(2.0f), glm::vec3(3.0f), glm::vec3(4.0f), glm::vec3(5.0f), glm::vec3(6.0f), glm::vec3(7.0f) };
	glm::vec3* a = new glm::vec3[8];
	a = move(data);

	/*
	while (counter < 3000)
	{
		if (triangle.mCenter.x >= 2.5f || triangle.mCenter.x <= -2.5f)
			change = -change;

		cout << "Triangle Center\n" << triangle.mCenter.x << "\t" << triangle.mCenter.y << endl;
		for (unsigned int i = 0; i < triangle.mSize; i++)
			cout << triangle.mPoints[i].x << "::" << triangle.mPoints[i].y << "\t";

		cout << "\nSquare Center\n" << square.mCenter.x << "\t" << square.mCenter.y << endl;
		for (unsigned int i = 0; i < square.mSize; i++)
			cout << square.mPoints[i].x << "::" << square.mPoints[i].y << "\t";

		if (GJK2D(square, triangle))
			cout << "Collision detected!!";

		cout << endl;

		triangle.translate(change);
		counter++;
	}
	*/
//	cout << (char)('A' + 1) << endl;

	return 0;
}