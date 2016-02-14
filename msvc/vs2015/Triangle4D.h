#ifndef TRIANGLE4D_H
#define TRIANGLE4D_H

#include "tri_util.h"

#ifdef BINARY_VOXELIZATION
#define TRIANGLE4D_SIZE 10 // just the vertices
#else
#define TRIANGLE4D_SIZE 22 // vertices + normal + vertex colors
#endif



//represents a triangle at a specific point in time
struct Triangle4D
{
	Triangle tri;
	float time;

	Triangle4D(): tri(Triangle()), time(0){}
	Triangle4D(Triangle triangle, float time): tri(triangle), time(time){}
};

#endif