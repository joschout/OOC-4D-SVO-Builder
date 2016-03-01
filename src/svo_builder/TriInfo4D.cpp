#include "TriInfo4D.h"

#include <stdio.h>  /* defines FILENAME_MAX */

#include <direct.h>
#define GetCurrentDir _getcwd

TriInfo4D::TriInfo4D(): end_time(0)
{
}

TriInfo4D::TriInfo4D(TriInfo triInfo, float end_time):
	triInfo3D(triInfo), mesh_bbox_transformed(AABox<vec4>(vec4(), vec4())),
	translation_direction(translation_direction), end_time(end_time)
{
}

TriInfo4D::~TriInfo4D()
{
}

