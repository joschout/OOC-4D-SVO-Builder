#ifndef TRANSLATIONHANDLER_H
#define TRANSLATIONHANDLER_H
#include <Vec.h>
#include "TransformationHandler.h"



class TranslationHandler : public TransformationHandler
{

private:
	vec3 translation_direction;
	float speed_factor = 1.0f;

public:
	TranslationHandler();
	TranslationHandler(const size_t gridsize, vec3 translation_direction);
	~TranslationHandler() override;
	void calculateTransformedBoundingBox(const TriInfo& triInfo, AABox<vec4>& mesh_bbox_transformed, float end_time) const;
	void transformAndStore(const TriInfo4D& tri_info, const Triangle& tri, vector<Buffer4D*>& buffers, const size_t nbOfPartitions) const override;
	static trimesh::vec3 translate(const trimesh::vec3 & point, const trimesh::vec3 & direction, const float amount);
	Triangle translate(const Triangle & triangle, const trimesh::vec3 & direction, const float amount) const;
};




#endif

