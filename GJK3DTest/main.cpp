#define _USE_MATH_DEFINES

#include <iostream>
#include <time.h>
#include <vector>
#include <cmath>


using namespace std;

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

	vec3 operator- (const vec3& val)
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

	vec3 operator- ()
	{
		return vec3(-x, -y, -z);
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
		return vec3(x + val.x, y + val.y, z + val.z);
	}

	vec3 sub(vec3 val)
	{
		return vec3(x - val.x, y - val.y, z - val.z);
	}

	template <typename T>
	vec3 mul(T val)
	{
		return vec3(x * val, y * val, z * val);
	}

	template <typename T>
	vec3 div(T val)
	{
		return vec3(x / val, y / val, z / val);
	}
};

/*
struct shape
{
	vec3 transform;

	vector<vec3> points;

	shape(vec3* points, vec3 trans = vec3()) : transform(trans) { this->points.push_back(*points); }

};
*/

void GJK(vector<vec3>& shapeA, vector<vec3>& shapeB);
vec3 supportFunction(vec3 dir, vector<vec3>& shape);

void GJK(vector<vec3>& shapeA, vector<vec3>& shapeB)
{
//	auto now = chrono::system_clock::now();
//	std::chrono::duration<float> elapsed_seconds = start - now;
//	float sec = elapsed_seconds.count();
	
	//Get a random direction vector
	float val = (float)(rand() % 360);
	cout << "Random degrees::\n" << val << endl;
	val *= M_PI / 180.0f;
	vec3 dir(cos(val), sin(val), 0.0f);
	cout << "Random direction::\n" << dir << endl << endl;

	//Find a point on both shapes A and B that each maximize the direction vector
	vec3 furthestShapePointA = supportFunction(dir, shapeA);
	//For Minkowski difference, invert the direction vector for Shape B
	vec3 furthestShapePointB = supportFunction(-dir, shapeB);
	cout << "Shape A::\n" << furthestShapePointA << endl;
	cout << "Shape B::\n" << furthestShapePointB << endl << endl;

	//Our point on the simplex will be the difference between both points
	vec3 simplexPointA = furthestShapePointA - furthestShapePointB;
	cout << "Simplex Point A::\n" << simplexPointA << endl << endl;

	dir = vec3() - simplexPointA;
	cout << "Direction to Origin::\n" << dir << endl;

	furthestShapePointA = supportFunction(dir, shapeA);
	furthestShapePointB = supportFunction(-dir, shapeB);
	cout << "Shape A::\n" << furthestShapePointA << endl;
	cout << "Shape B::\n" << furthestShapePointB << endl << endl;

	vec3 simplexPointB = furthestShapePointA - furthestShapePointB;
	cout << "Simplex Point B::\n" << simplexPointB << endl << endl;

	vec3 dirAO = -simplexPointA;
	vec3 dirAB = simplexPointB - simplexPointA;
	
	vec3 ABxAO = vec3::cross(dirAB, dirAO);
	dir = vec3::cross(vec3::cross(dirAB, dirAO), dirAB);

	furthestShapePointA = supportFunction(dir, shapeA);
	furthestShapePointB = supportFunction(-dir, shapeB);
	cout << "Shape A::\n" << furthestShapePointA << endl;
	cout << "Shape B::\n" << furthestShapePointB << endl << endl;

	vec3 simplexPointC = furthestShapePointA - furthestShapePointB;
	cout << "Simplex Point C::\n" << simplexPointC << endl << endl;

	vec3 dirAC = simplexPointC - simplexPointA;

	vec3 ABperp = vec3::cross(vec3::cross(dirAB, dirAC), dirAC);
	vec3 ACperp = vec3::cross(vec3::cross(dirAC, dirAB), dirAB);

	bool abCheck = vec3::dot(ABperp, dirAO) < 0.0f;
	bool acCheck = vec3::dot(ACperp, dirAO) < 0.0f;

	if (abCheck && abCheck)
		cout << "ORIGIN IN SIMPLEX" << endl << endl;
}

vec3 supportFunction(vec3 dir, vector<vec3>& shape)
{
	float maxDotProd = vec3::dot(dir, shape[0]);
	vec3 furthestPoint = shape[0];

	for (unsigned int i = 1; i < shape.size(); i++)
	{
		float val = vec3::dot(dir, shape[i]);
		if (val > maxDotProd)
		{
			furthestPoint = shape[i];
			maxDotProd = val;
		}
	}
		
	return furthestPoint;
}

int main()
{
	srand((unsigned int)time(NULL));

	vec3 v(10.0f);
	vec3 a = v;
	a = -a;
	a = (a - v) / 5;

	cout << a << endl;

	vec3 x(3.0f, 2.0f, 1.0f);
	vec3 y(4.0f);

	cout << vec3::dot(x, y) << endl;
	cout << vec3::cross(x, y) << endl;

	
	vector<vec3> square = {
		vec3(-1.0f,-1.0f, 0.0f),
		vec3( 1.0f,-1.0f, 0.0f),
		vec3( 1.0f, 1.0f, 0.0f),
		vec3(-1.0f, 1.0f, 0.0f)
	};

	vector<vec3> triangle = {
		vec3(0.5f, -0.5f, 0.0f),
		vec3(2.5f, -0.5f, 0.0f),
		vec3(1.5f,  0.5f, 0.0f)
	};
	
	GJK(square, triangle);
	
	return 0;
}