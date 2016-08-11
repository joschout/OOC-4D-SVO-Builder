#include "TransformationHandler.h"


TransformationHandler::TransformationHandler(size_t gridsize_T): gridsize_T(gridsize_T)
{
}

void TransformationHandler::storeTriangleInPartitionBuffers(Triangle4D transformed_tri, vector<Buffer4D*>& buffers, const size_t nbOfPartitions) const
{
//	cout << "transformed 4D tri: " << endl
//		<< "  v0: " << transformed_tri.tri.v0 << endl
//		<< "  v1: " << transformed_tri.tri.v1 << endl
//		<< "  v2: " << transformed_tri.tri.v2 << endl
//		<< "  time: " << transformed_tri.time << endl;
	AABox<vec4> bbox4D_transformed_tri = computeBoundingBox(transformed_tri);

	bool triangleIsInAPartition = false;

	for (auto j = 0; j < nbOfPartitions; j++) { // Test against all partitions
		bool triangleIsInThisPartition 
			= buffers[j]->processTriangle(transformed_tri, bbox4D_transformed_tri);

		triangleIsInAPartition =
			triangleIsInAPartition || triangleIsInThisPartition;
		/*				cout << "triangle: "
		<< "v0: "<< translated_t_time.tri.v0
		<< " v1: " << translated_t_time.tri.v1
		<< " v2: " << translated_t_time.tri.v2
		<< " is in partition (1/0): " << isInPartition << endl;*/
	}

	if(! triangleIsInAPartition)
	{
		cout << "ERROR: a transformed triangle is not a part of any partition!" << endl;
		std::cout << "Press ENTER to continue...";
		cin.get();
	}
}


