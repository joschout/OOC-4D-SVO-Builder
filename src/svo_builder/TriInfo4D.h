#ifndef EXTENDEDTRIINFO_H
#define EXTENDEDTRIINFO_H

#include <tri_tools.h>

class TriInfo4D
{
public:
	TriInfo triInfo3D;
	AABox<vec4> mesh_bbox_transl;
	vec3 translation_direction;
	float end_time;

	TriInfo4D();
	TriInfo4D(TriInfo triInfo, vec3 translation_direction, float end_time);
	~TriInfo4D();

};



#endif

