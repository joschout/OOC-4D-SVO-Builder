#ifndef EXTENDEDTRIINFO_H
#define EXTENDEDTRIINFO_H

#include <tri_tools.h>

class ExtendedTriInfo
{
public:
	TriInfo triInfo3D;
	AABox<vec4> mesh_bbox_transl;
	vec3 translation_direction;
	float end_time;

	ExtendedTriInfo();
	ExtendedTriInfo(TriInfo triInfo, vec3 translation_direction, float end_time);
	~ExtendedTriInfo();

	static int parseTri3DHeader(std::string filename, TriInfo &t);
};



#endif

