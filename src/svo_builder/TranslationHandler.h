#ifndef TRANSLATIONHANDLER_H
#define TRANSLATIONHANDLER_H

#include <Vec.h>

inline trimesh::vec3 translate(const trimesh::vec3 &point, const trimesh::vec3 &direction, const float amount)
{
	trimesh::vec3 scaled_direction = amount * direction;
	trimesh::vec3 translated_point = point + scaled_direction;
	return translated_point;
}

inline Triangle translate(const Triangle &triangle, const trimesh::vec3 &direction, const float amount)
{
	Triangle temp;
	temp.v0 = translate(triangle.v0, direction, amount);
	temp.v1 = translate(triangle.v1, direction, amount);
	temp.v2 = translate(triangle.v2, direction, amount);
	return temp;
}

#endif

