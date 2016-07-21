#ifndef TRIINFO4D_H
#define TRIINFO4D_H
#include <tri_tools.h>

class TriInfo4D
{
public:
	TriInfo triInfo3D;
	AABox<vec4> mesh_bbox_transformed;
	//vec3 translation_direction;
	//float end_time;

	TriInfo4D();
	TriInfo4D(TriInfo triInfo, float end_time);
	~TriInfo4D();

};



#endif

