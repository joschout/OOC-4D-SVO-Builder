#ifndef ROTATIONHANDLER_H
#define ROTATIONHANDLER_H
#include <TriReader.h>
#include <Vec.h>
#include <math.h>
#include "TransformationHandler.h"


enum RotationAxis{ X_AXIS, Y_AXIS, Z_AXIS};

class RotationHandler : public TransformationHandler
{
public:
	RotationAxis rotation_axis;

	RotationHandler();
	RotationHandler(const size_t gridsize_T, RotationAxis rotation_axis);
	~RotationHandler() override;
	void calculateTransformedBoundingBox(const TriInfo& triInfo, AABox<vec4>& mesh_bbox_transformed, float end_time) const;
	
	typedef Triangle(RotationHandler::*fptr_triangleRotation)(const Triangle&, const float) const;
	fptr_triangleRotation getTriangleRotationFunction() const;

	typedef vec3(RotationHandler::*fptr_pointRotation)(const vec3&, const float)const;
	fptr_pointRotation getPointRotationFunction() const;
	void transformAndStore(const TriInfo4D& tri_info, const Triangle& tri, vector<Buffer4D*>& buffers, const size_t nbOfPartitions) const override;
	trimesh::vec3 rotateX(const trimesh::vec3 & point, const float angle) const;
	trimesh::vec3 rotateY(const trimesh::vec3 & point, const float angle) const;
	trimesh::vec3 rotateZ(const trimesh::vec3 & point, const float angle) const;
	Triangle rotateX(const Triangle & triangle, const float angle) const;
	Triangle rotateY(const Triangle & triangle, const float angle) const;
	Triangle rotateZ(const Triangle & triangle, const float angle) const;
};



#endif