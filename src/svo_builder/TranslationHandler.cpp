#include "TranslationHandler.h"

TranslationHandler::TranslationHandler(): TransformationHandler(1)
{
}

TranslationHandler::TranslationHandler(const size_t gridsize, vec3 translation_direction): TransformationHandler(gridsize), translation_direction(translation_direction)
{
}

TranslationHandler::~TranslationHandler()
{
}

void TranslationHandler::calculateTransformedBoundingBox(const TriInfo& triInfo, AABox<vec4>& mesh_bbox_transformed, float end_time) const
{
	AABox<vec3> start_box = triInfo.mesh_bbox;

	vec3 translated_min = translate(start_box.min, translation_direction, speed_factor * end_time);
	vec3 translated_max = translate(start_box.max, translation_direction, speed_factor * end_time);

	mesh_bbox_transformed.min[0] = min(start_box.min[0], min(translated_min[0], translated_max[0]));
	mesh_bbox_transformed.min[1] = min(start_box.min[1], min(translated_min[1], translated_max[1]));
	mesh_bbox_transformed.min[2] = min(start_box.min[2], min(translated_min[2], translated_max[2]));

	mesh_bbox_transformed.max[0] = max(start_box.max[0], max(translated_min[0], translated_max[0]));
	mesh_bbox_transformed.max[1] = max(start_box.max[1], max(translated_min[1], translated_max[1]));
	mesh_bbox_transformed.max[2] = max(start_box.max[2], max(translated_min[2], translated_max[2]));

	mesh_bbox_transformed.min[3] = 0;
	mesh_bbox_transformed.max[3] = end_time;
}

void TranslationHandler::transformAndStore(const TriInfo4D& tri_info, const Triangle& tri, vector<Buffer4D*>& buffers, const size_t nbOfPartitions) const
{
	float unitlength_time = (tri_info.mesh_bbox_transformed.max[3] - tri_info.mesh_bbox_transformed.min[3]) / (float)gridsize;
	for (float time = 0; time < tri_info.end_time; time = time + unitlength_time)
	{
		//cout << endl << "time point:" << time << endl;

		Triangle translated_tri = translate(tri, translation_direction, speed_factor * time);
		Triangle4D translated_tri_time = Triangle4D(translated_tri, time);
		AABox<vec4> bbox4D = computeBoundingBox(translated_tri_time);
		for (auto j = 0; j < nbOfPartitions; j++) { // Test against all partitions
			bool isInPartition = buffers[j]->processTriangle(translated_tri_time, bbox4D);

			/*				cout << "triangle: "
			<< "v0: "<< translated_t_time.tri.v0
			<< " v1: " << translated_t_time.tri.v1
			<< " v2: " << translated_t_time.tri.v2
			<< " is in partition (1/0): " << isInPartition << endl;*/
		}
	}
}

trimesh::vec3 TranslationHandler::translate(const trimesh::vec3 &point, const trimesh::vec3 &direction, const float amount)
{
	trimesh::vec3 scaled_direction = amount * direction;
	trimesh::vec3 translated_point = point + scaled_direction;
	return translated_point;
}

Triangle TranslationHandler::translate(const Triangle &triangle, const trimesh::vec3 &direction, const float amount) const
{
	Triangle temp;
	temp.v0 = translate(triangle.v0, direction, amount);
	temp.v1 = translate(triangle.v1, direction, amount);
	temp.v2 = translate(triangle.v2, direction, amount);
	return temp;
}

