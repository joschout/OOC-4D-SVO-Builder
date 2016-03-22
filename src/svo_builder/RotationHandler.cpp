#include "RotationHandler.h"

RotationHandler::RotationHandler(): TransformationHandler(1)
{
	rotation_axis = X_AXIS;
}

RotationHandler::RotationHandler(const size_t gridsize, RotationAxis rotation_axis): TransformationHandler(gridsize), rotation_axis(rotation_axis)
{
}

RotationHandler::~RotationHandler()
{
}

void RotationHandler::calculateTransformedBoundingBox(const TriInfo& triInfo, AABox<vec4>& mesh_bbox_transformed, float end_time) const
{
	mesh_bbox_transformed.min[3] = 0;
	mesh_bbox_transformed.max[3] = end_time;
	
	mesh_bbox_transformed.min[0] = triInfo.mesh_bbox.min[0];
	mesh_bbox_transformed.max[0] = triInfo.mesh_bbox.max[0];
	mesh_bbox_transformed.min[1] = triInfo.mesh_bbox.min[1];
	mesh_bbox_transformed.max[1] = triInfo.mesh_bbox.max[1];
	mesh_bbox_transformed.min[2] = triInfo.mesh_bbox.min[2];
	mesh_bbox_transformed.max[2] = triInfo.mesh_bbox.max[2];

	//bounding box has six corners
	vec3 p_min = triInfo.mesh_bbox.min;
	vec3 p_max = triInfo.mesh_bbox.max;

	// 1 max, 2 min
	vec3 pa = vec3(p_max[0], p_min[1], p_min[2]);
	vec3 pb = vec3(p_min[0], p_max[1], p_min[2]);
	vec3 pc = vec3(p_min[0], p_min[1], p_max[2]);
	// 2 max, 1 min
	vec3 pd = vec3(p_min[0], p_max[1], p_max[2]);
	vec3 pe = vec3(p_max[0], p_min[1], p_max[2]);
	vec3 pf = vec3(p_max[0], p_max[1], p_min[2]);


	float unit_angle_degrees = 360 / (float)(gridsize - 1);

	RotationHandler::fptr_pointRotation rotationFunction = getPointRotationFunction();

	for (int i = 0; i <= gridsize; i = i + 1)
	{
		
		vec3 rotated_min = (this->*rotationFunction)(p_min, i * unit_angle_degrees);
		vec3 rotated_max = (this->*rotationFunction)(p_max, i * unit_angle_degrees);
		vec3 rotated_pa = (this->*rotationFunction)(pa, i * unit_angle_degrees);
		vec3 rotated_pb = (this->*rotationFunction)(pb, i * unit_angle_degrees);
		vec3 rotated_pc = (this->*rotationFunction)(pc, i * unit_angle_degrees);
		vec3 rotated_pd = (this->*rotationFunction)(pd, i * unit_angle_degrees);
		vec3 rotated_pe = (this->*rotationFunction)(pe, i * unit_angle_degrees);
		vec3 rotated_pf = (this->*rotationFunction)(pf, i * unit_angle_degrees);


		for (int axis = 0; i < 3; i++) {
			float temp_min_1 = min(rotated_min[axis], min(rotated_max[axis], min(rotated_pa[axis], rotated_pb[axis])));
			float temp_min_2 = min(rotated_pc[axis], min(rotated_pd[axis], min(rotated_pe[axis], rotated_pf[axis])));
			mesh_bbox_transformed.min[axis] = min(mesh_bbox_transformed.min[axis], min(temp_min_1, temp_min_2));

			float temp_max_1 = max(rotated_min[axis], max(rotated_max[axis], max(rotated_pa[axis], rotated_pb[axis])));
			float temp_max_2 = max(rotated_pc[axis], max(rotated_pd[axis], max(rotated_pe[axis], rotated_pf[axis])));
			mesh_bbox_transformed.max[axis] = max(mesh_bbox_transformed.max[axis], max(temp_max_1, temp_max_2));
		}

	}

	cout << "MIN: x: " << mesh_bbox_transformed.min[0]
		<< ", y: " << mesh_bbox_transformed.min[1]
		<< ", z: " << mesh_bbox_transformed.min[2]
		<< ", t: " << mesh_bbox_transformed.min[3] << endl;
	cout << "MAX: x: " << mesh_bbox_transformed.max[0]
		<< ", y: " << mesh_bbox_transformed.max[1]
		<< ", z: " << mesh_bbox_transformed.max[2]
		<< ", t: " << mesh_bbox_transformed.max[3] << endl << endl;

}

RotationHandler::fptr_triangleRotation RotationHandler::getTriangleRotationFunction() const
{
	switch (rotation_axis)
	{
	case X_AXIS:
		return &RotationHandler::rotateX;
	case Y_AXIS:
		return &RotationHandler::rotateY;
	case Z_AXIS:
		return &RotationHandler::rotateZ;
	}
	return &RotationHandler::rotateX;
}

RotationHandler::fptr_pointRotation RotationHandler::getPointRotationFunction() const
{
	switch (rotation_axis)
	{
	case X_AXIS:
		return &RotationHandler::rotateX;
	case Y_AXIS:
		return &RotationHandler::rotateY;
	case Z_AXIS:
		return &RotationHandler::rotateZ;
	}
	return &RotationHandler::rotateX;
}

void RotationHandler::transformAndStore(const TriInfo4D& tri_info, const Triangle& tri, vector<Buffer4D*>& buffers, const size_t nbOfPartitions) const
{

	float unitlength_time = (tri_info.mesh_bbox_transformed.max[3] - tri_info.mesh_bbox_transformed.min[3]) / (float)gridsize;
	float unit_angle_degrees = 360 / (float)(gridsize - 1);

	 RotationHandler::fptr_triangleRotation rotationFunction = getTriangleRotationFunction();

	 cout << endl;

	for (int i = 0; i < gridsize; i = i + 1)
	{
		//	cout << endl << "time point:" << time << endl;
		Triangle rotated_tri = (this->*rotationFunction)(tri, i *unit_angle_degrees);

		//Triangle rotated_tri = rotateX(tri, unit_angle_degrees);
		float time = i * unitlength_time;
		Triangle4D rotated_tri_time = Triangle4D(rotated_tri, time);

		storeTriangleInPartitionBuffers(rotated_tri_time, buffers, nbOfPartitions);
	}
}


// rotates about the x axis in a counter clockwise direction.
trimesh::vec3 RotationHandler::rotateX(const trimesh::vec3 &point, const float angle) const
{
	float radAngle = angle * M_PI / 180;
	float x_old = point[0];
	float y_old = point[1];
	float z_old = point[2];

	float x_new = x_old;
	float y_new = cos(radAngle) * y_old - sin(radAngle) * z_old;
	float z_new = sin(radAngle) * y_old + cos(radAngle) * z_old;

	return trimesh::vec3(x_new, y_new, z_new);
}

// rotates about the y axis in a counter clockwise direction.
trimesh::vec3 RotationHandler::rotateY(const trimesh::vec3 &point, const float angle) const
{
	float radAngle = angle * M_PI / 180;
	float x_old = point[0];
	float y_old = point[1];
	float z_old = point[2];

	float x_new = cos(radAngle) * x_old + sin(radAngle) * z_old;
	float y_new = y_old;
	float z_new = -sin(radAngle) * x_old + cos(radAngle) * z_old;

	return trimesh::vec3(x_new, y_new, z_new);
}

// rotates about the z axis in acounter clockwise direction.
trimesh::vec3 RotationHandler::rotateZ(const trimesh::vec3 &point, const float angle) const
{
	float radAngle = angle * M_PI / 180;
	float x_old = point[0];
	float y_old = point[1];
	float z_old = point[2];

	float x_new = cos(radAngle) * x_old - sin(radAngle) * y_old;
	float y_new = sin(radAngle) * x_old + cos(radAngle) * y_old;
	float z_new = z_old;

	return trimesh::vec3(x_new, y_new, z_new);
}

Triangle RotationHandler::rotateX(const Triangle &triangle, const float angle) const
{
	Triangle temp;
	temp.v0 = rotateX(triangle.v0, angle);
	temp.v1 = rotateX(triangle.v1, angle);
	temp.v2 = rotateX(triangle.v2, angle);
	return temp;
}

Triangle RotationHandler::rotateY(const Triangle &triangle, const float angle) const
{
	Triangle temp;
	temp.v0 = rotateY(triangle.v0, angle);
	temp.v1 = rotateY(triangle.v1, angle);
	temp.v2 = rotateY(triangle.v2, angle);
	return temp;
}

Triangle RotationHandler::rotateZ(const Triangle &triangle, const float angle) const
{
	Triangle temp;
	temp.v0 = rotateZ(triangle.v0, angle);
	temp.v1 = rotateZ(triangle.v1, angle);
	temp.v2 = rotateX(triangle.v2, angle);
	return temp;
}
