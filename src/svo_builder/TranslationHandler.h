#ifndef TRANSLATIONHANDLER_H
#define TRANSLATIONHANDLER_H

#include <Vec.h>

inline trimesh::vec3 translate(const trimesh::vec3 point, const trimesh::vec3 direction, const float amount)
{
	trimesh::vec3 scaled_direction = amount * direction;
	trimesh::vec3 translated_point = point + scaled_direction;
	return translated_point;
}

#endif

