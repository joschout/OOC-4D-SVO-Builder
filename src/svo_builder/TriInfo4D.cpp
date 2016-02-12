#include "TriInfo4D.h"
#include "TranslationHandler.h"


#include <stdio.h>  /* defines FILENAME_MAX */

#include <direct.h>
#define GetCurrentDir _getcwd

TriInfo4D::TriInfo4D(): end_time(0)
{
}

TriInfo4D::TriInfo4D(TriInfo triInfo, vec3 translation_direction, float end_time):
	triInfo3D(triInfo), mesh_bbox_transl(AABox<vec4>(vec4(), vec4())),
	translation_direction(translation_direction), end_time(end_time)
{

	AABox<vec3> start_box = triInfo3D.mesh_bbox;
	
	vec3 translated_min = translate(start_box.min, translation_direction, end_time);
	vec3 translated_max = translate(start_box.max, translation_direction, end_time);

	mesh_bbox_transl.min[0] = min(start_box.min[0], min(translated_min[0], translated_max[0]));
	mesh_bbox_transl.min[1] = min(start_box.min[1], min(translated_min[1], translated_max[1]));
	mesh_bbox_transl.min[2] = min(start_box.min[2], min(translated_min[2], translated_max[2]));

	mesh_bbox_transl.max[0] = max(start_box.max[0], max(translated_min[0], translated_max[0]));
	mesh_bbox_transl.max[1] = max(start_box.max[1], max(translated_min[1], translated_max[1]));
	mesh_bbox_transl.max[2] = max(start_box.max[2], max(translated_min[2], translated_max[2]));

	mesh_bbox_transl.min[3] = 0;
	mesh_bbox_transl.max[3] = end_time;

}

TriInfo4D::~TriInfo4D()
{
}

