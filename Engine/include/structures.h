#ifndef STRUCTURES_H
#define STRUCTURES_H


//alignment constants
const int ALIGN_CENTER = 0;
const int ALIGN_LEFT = 1;
const int ALIGN_RIGHT = 2;
const int ALIGN_TOP = 3;
const int ALIGN_BOTTOM = 4;

#include <vector>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define MATH_PI 3.14159265358979323846264f

struct RGBA_Color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

//vector2
typedef struct vector2 {
	double x;
	double y;
	double dot(vector2 v) {
		return v.x*x + v.y*y;
	}
	double cross(vector2 v) {
		return (x * v.y) - (y * v.x);
	}
	vector2 normal() {
		return {-y, x};
	}
	vector2 normalize() {
		double magnitude = sqrt(x * x + y * y);
		vector2 n = { x / magnitude, y / magnitude };
		return n;
	}
	vector2 invert() {
		return {-x, -y};
	}
	double magnitude() {
		return sqrt(x * x + y * y);
	}
}vector2;

/*struct matrix2 {
	double a, b, c, d;
	vector2 dot(vector2 v) {
		return { a * v.x + b * v.y, c * v.x + d * v.y };
	};
	matrix2 invert() {
		double k = 1 / (a * d - b * c);
		return { d * k, -b * k, -c * k, a * k };
	};
};*/

struct TransformStruct {
	vector2 position;
	vector2 scale;
	double rotation;
};

struct P_Array {
	vector2 array[512];
	int array_len = 0;
	int size() {
		return array_len;
	}
	vector2& operator[](std::size_t idx) { return array[idx]; }
	// copy assignment
	P_Array& operator=(const P_Array& other)
	{
		// Guard self assignment
		if (this == &other)
			return *this;

		array_len = other.array_len;
		memcpy(array, other.array, sizeof(vector2) * array_len);
		return *this;
	}
	P_Array& operator=(std::vector <vector2>& other)
	{

		array_len = other.size();
		for (int i = 0; i < other.size(); i++) {
			array[i] = other[i];
		}
		return *this;
	}
};


#endif