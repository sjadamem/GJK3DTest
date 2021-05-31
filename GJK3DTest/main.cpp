#include <iostream>
#include <chrono>
#include <time.h>
#include <vector>

using namespace std;

auto start = chrono::system_clock::now();

void GJK();

struct vec3
{
public:
	float x, y, z;

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

	static float dot(vec3& a, vec3& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	static vec3 cross(vec3& a, vec3& b)
	{
		return vec3(
			a.y * b.z - a.z * b.y, 
			a.z * b.x - a.x * b.z, 
			a.x * b.y - a.y * b.x
		);
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

void GJK()
{
	auto now = chrono::system_clock::now();
	std::chrono::duration<float> elapsed_seconds = start - now;
	float sec = elapsed_seconds.count();
	
	float val = (float)rand();
	cout << val << "\n" << vec3(cos(val), sin(val), 0.0f) << endl;
}

int main()
{
	vector<vec3> square;
	vector<vec3> triangle;

	square = {
		vec3(-0.5f, -0.5f, 0.0f),
		vec3( 0.5f, -0.5f, 0.0f),
		vec3( 0.5f,  0.5f, 0.0f),
		vec3(-0.5f,  0.5f, 0.0f)
	};

	triangle = {
		vec3( 0.0f, -0.5f, 0.0f),
		vec3( 1.0f, -0.5f, 0.0f),
		vec3( 0.0f,  0.5f, 0.0f)
	};

	vec3 v(10.0f);
	vec3 a = v;
	a = -a;
	a = (a - v) / 5;

	cout << a << endl;


	vec3 x(3.0f, 2.0f, 1.0f);
	vec3 y(4.0f);

	cout << vec3::dot(x, y) << endl;
	cout << vec3::cross(x, y) << endl;

	GJK();
	
	return 0;
}