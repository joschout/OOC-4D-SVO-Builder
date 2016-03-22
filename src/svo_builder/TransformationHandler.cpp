#include "TransformationHandler.h"


TransformationHandler::TransformationHandler(size_t gridsize): gridsize(gridsize)
{
}

void TransformationHandler::storeTriangleInPartitionBuffers(Triangle4D transformed_tri, vector<Buffer4D*>& buffers, const size_t nbOfPartitions) const
{
	AABox<vec4> bbox4D_transformed_tri = computeBoundingBox(transformed_tri);

	for (auto j = 0; j < nbOfPartitions; j++) { // Test against all partitions
		bool isInPartition = buffers[j]->processTriangle(transformed_tri, bbox4D_transformed_tri);

		/*				cout << "triangle: "
		<< "v0: "<< translated_t_time.tri.v0
		<< " v1: " << translated_t_time.tri.v1
		<< " v2: " << translated_t_time.tri.v2
		<< " is in partition (1/0): " << isInPartition << endl;*/
	}
}


