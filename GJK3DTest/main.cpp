#include <iostream>
#include <chrono>
#include <time.h>
#include <vector>

using namespace std;

auto start = chrono::system_clock::now();

void GJK();

struct vec3
{
	float x, y, z;

	vec3(float x, float y, float z) : x(x), y(y), z(z) {};
	vec3(float val) : x(val), y(val), z(val) {}

	friend ostream &operator<<(ostream &output, const vec3 &v) {
		output << "X : " << v.x << "|Y : " << v.y << "|Z : " << v.z;
		return output;
	}
};

void GJK()
{
	auto now = chrono::system_clock::now();
	std::chrono::duration<float> elapsed_seconds = start - now;
	float sec = elapsed_seconds.count();
	
	vec3(cos((float(rand()))), sin((float(rand()))), 0.0f);
}

int main()
{
	vector<vec3> square;
	vector<vec3> triangle;

	vec3 v(10.0f);
	cout << v << endl;

	system("pause");
	return 0;
}