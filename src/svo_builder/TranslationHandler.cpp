#include "TranslationHandler.h"

TranslationHandler::TranslationHandler(): TransformationHandler(1)
{
}

TranslationHandler::TranslationHandler(const size_t gridsize_T, vec3 translation_direction): TransformationHandler(gridsize_T), translation_direction(translation_direction)
{
}

TranslationHandler::~TranslationHandler()
{
}

void TranslationHandler::calculateTransformedBoundingBox(TriInfo4D& triInfo4D, float end_time) const
{
	AABox<vec3> start_box = triInfo4D.triInfo3D.mesh_bbox;


//	vec3& p0 = start_box.min;
//	vec3& p1 = start_box.max;
//
//	vec3 p0_t = translate(p0, translation_direction, speed_factor * end_time);
//	vec3 p1_t = translate(p1, translation_direction, speed_factor * end_time);
//
//	vec4 p0_tot;
//	vec4 p1_tot;
//
//	p0_tot[0] = min(min(p0[0], p1[0]), min(p0_t[0], p1_t[0]));
//	p0_tot[1] = min(min(p0[1], p1[1]), min(p0_t[1], p1_t[1]));
//	p0_tot[2] = min(min(p0[2], p1[2]), min(p0_t[2], p1_t[2]));
//	p0_tot[3] = 0;
//
//	p1_tot[0] = max(max(p0[0], p1[0]), max(p0_t[0], p1_t[0]));
//	p1_tot[1] = max(max(p0[1], p1[1]), max(p0_t[1], p1_t[1]));
//	p1_tot[2] = max(max(p0[2], p1[2]), max(p0_t[2], p1_t[2]));
//	p1_tot[3] = end_time;




	vec3 translated_min = translate(start_box.min, translation_direction, speed_factor * end_time);
	vec3 translated_max = translate(start_box.max, translation_direction, speed_factor * end_time);

	triInfo4D.mesh_bbox_transformed.min[0] = min(start_box.min[0], min(translated_min[0], translated_max[0]));
	triInfo4D.mesh_bbox_transformed.min[1] = min(start_box.min[1], min(translated_min[1], translated_max[1]));
	triInfo4D.mesh_bbox_transformed.min[2] = min(start_box.min[2], min(translated_min[2], translated_max[2]));

	triInfo4D.mesh_bbox_transformed.max[0] = max(start_box.max[0], max(translated_min[0], translated_max[0]));
	triInfo4D.mesh_bbox_transformed.max[1] = max(start_box.max[1], max(translated_min[1], translated_max[1]));
	triInfo4D.mesh_bbox_transformed.max[2] = max(start_box.max[2], max(translated_min[2], translated_max[2]));

	triInfo4D.mesh_bbox_transformed.min[3] = 0;
	triInfo4D.mesh_bbox_transformed.max[3] = end_time;
}

void TranslationHandler::transformAndStore(const TriInfo4D& tri_info, const Triangle& tri, vector<Buffer4D*>& buffers, const size_t nbOfPartitions) const
{
	float unitlength_time = (tri_info.mesh_bbox_transformed.max[3] - tri_info.mesh_bbox_transformed.min[3]) / (float)gridsize_T;
	for (float time = tri_info.mesh_bbox_transformed.min[3]; time < tri_info.mesh_bbox_transformed.max[3]; time = time + unitlength_time)
	{
//		if(time >= tri_info.mesh_bbox_transformed.min[3] + unitlength_time * 32 && time < tri_info.mesh_bbox_transformed.min[3] + unitlength_time * 48)
//		{
//			cout << "hier loopt het mis" << endl;
//		}

		//cout << endl << "time point:" << time << endl;
		Triangle translated_tri = translate(tri, translation_direction, speed_factor * time);
		Triangle4D translated_tri_time = Triangle4D(translated_tri, time);
		
//		cout << "translated 4D tri: " << endl
//			<< "  v0: " << translated_tri_time.tri.v0 << endl
//			<< "  v1: " << translated_tri_time.tri.v1 << endl
//			<< "  v2: " << translated_tri_time.tri.v2 << endl
//			<< "  time: " << translated_tri_time.time << endl;

		storeTriangleInPartitionBuffers(translated_tri_time, buffers, nbOfPartitions);
	}
}

trimesh::vec3 TranslationHandler::translate(const trimesh::vec3 &point, const trimesh::vec3 &direction, const float amount)
{
	trimesh::vec3 scaled_direction = amount * direction;
	trimesh::vec3 translated_point = point + scaled_direction;
	return translated_point;
}

Triangle TranslationHandler::translate(const Triangle &triangle, const trimesh::vec3 &direction, const float amount)
{
	Triangle temp(triangle);
	temp.v0 = translate(triangle.v0, direction, amount);
	temp.v1 = translate(triangle.v1, direction, amount);
	temp.v2 = translate(triangle.v2, direction, amount);
	return temp;
}

