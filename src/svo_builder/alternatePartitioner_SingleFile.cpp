#include "alternatePartitioner_SingleFile.h"
#include "TriPartitioningInfo4D.h"
#include "TranslationHandler.h"

alternatePartitioner_SingleFile::alternatePartitioner_SingleFile() :
	alternatePartitioner_Interface(1, 1, 4)
{
}

alternatePartitioner_SingleFile::alternatePartitioner_SingleFile(size_t gridsize_S, size_t gridsize_T, size_t nbOfDimensions):
	alternatePartitioner_Interface(gridsize_S, gridsize_T, nbOfDimensions)
{
}

TriPartitioningInfo4D alternatePartitioner_SingleFile::partitionTriangleModel(TriInfo4D& extended_tri_info, size_t voxel_memory_limit, TransformationHandler* transformation_handler)
{
	//estimate the amount of triangle partitions needed for voxelization
	//NOTE: the estimation is hardcoded for 3D trees
	size_t nbOfTrianglePartitions = estimateNumberOfPartitions(voxel_memory_limit);
	cout << "Partitioning data into " << nbOfTrianglePartitions << " partitions ... " << endl; cout.flush();

	if (data_out)
	{
		data_writer_ptr->writeToFile_endl("number Of partitions: " + to_string(nbOfTrianglePartitions));
	}

	//partition the  triangle mesh
	TriPartitioningInfo4D trianglePartition_info = partition(extended_tri_info, transformation_handler);
	cout << "done." << endl;

	return trianglePartition_info;
}

// Partition the mesh referenced by tri_info into n triangle partitions for gridsize,
// and store information about the partitioning in trip_info
TriPartitioningInfo4D alternatePartitioner_SingleFile::partition(const TriInfo4D& tri_info, TransformationHandler* transformation_handler)
{
	// Open tri_data stream
	// the reader knows how many triangles there are in the model,
	// the reader is given input_buffersize as buffersize for buffering read triangles
	partitioning_io_input_timer.start();	// TIMING
	const std::string tridata_filename = tri_info.triInfo3D.base_filename + string(".tridata");
	TriReader tridataReader = TriReader(tridata_filename, tri_info.triInfo3D.n_triangles, input_buffersize);
	partitioning_io_input_timer.stop();		// TIMING

	partitioning_algorithm_timer.start();	// TIMING
	// Create Mortonbuffers: we will have one buffer per partition
	vector<Buffer4D*> buffers;
	createBuffers(tri_info.mesh_bbox_transformed, tri_info.triInfo3D.base_filename, buffers);

	//for each triangle
	while (tridataReader.hasNext()) {
		Triangle tri;
		partitioning_algorithm_timer.stop();	// TIMING
		partitioning_io_input_timer.start();	// TIMING
		tridataReader.getTriangle(tri);
		partitioning_io_input_timer.stop();		// TIMING
		partitioning_algorithm_timer.start();	// TIMING

		//transform the triangle to the different points in time and store those triangles in the buffers

		//=============== TIJDELIJK


		const float& end_time = tri_info.mesh_bbox_transformed.max[3];
		const float& start_time = tri_info.mesh_bbox_transformed.min[3];
		float unitlength_time = (end_time - start_time) / (float)gridsize_T;
		float time = 0;

		while(time < end_time)
		{
			partitioning_algorithm_timer.stop();	// TIMING
			partitioning_total_timer.stop();		// TIMING

			vec3 translation_direction = vec3(1.0, 1.0, 1.0);
			translation_direction = normalize(translation_direction);
			float speed_factor = 1.0f;
			

			Triangle translated_tri = TranslationHandler::translate(tri, translation_direction, speed_factor * time);

			Triangle4D translated_tri_time = Triangle4D(translated_tri, time);

			partitioning_total_timer.start();		// TIMING
			partitioning_algorithm_timer.start();	// TIMING

			storeTriangleInPartitionBuffers(translated_tri_time, buffers, nbOfPartitions);

			time += unitlength_time;
		}
		//===============================

//		transformation_handler->transformAndStore(tri_info, tri, buffers, nbOfPartitions);
	}
	partitioning_algorithm_timer.stop();	// TIMING

	partitioning_io_output_timer.start();	// TIMING
	TriPartitioningInfo4D trip_info = createTriPartitioningInfoHeader(tri_info, buffers);
	partitioning_io_output_timer.stop();	// TIMING

	deleteBuffers(buffers);

	return trip_info;
}

TriPartitioningInfo4D alternatePartitioner_SingleFile::createTriPartitioningInfoHeader(const TriInfo4D tri_info, vector<Buffer4D*>& buffers) const
{
	TriPartitioningInfo4D trip_info = TriPartitioningInfo4D(tri_info);

	auto nbOfTriangles_incl_transf = gridsize_T * tri_info.triInfo3D.n_triangles;
	trip_info.n_triangles = nbOfTriangles_incl_transf;

	// Collect nbOfTriangles of each partition
	trip_info.nbOfTrianglesPerPartition.resize(nbOfPartitions);
	for (size_t j = 0; j < nbOfPartitions; j++) {
		trip_info.nbOfTrianglesPerPartition[j] = buffers[j]->n_triangles;
	}

	// Write trip header
	trip_info.base_filename
		= tri_info.triInfo3D.base_filename
		+ string("_S") + val_to_string(gridsize_S)
		+ string("_T") + val_to_string(gridsize_T)
		+ string("_P") + val_to_string(nbOfPartitions);
	std::string header = trip_info.base_filename + string(".trip");
	trip_info.gridsize_S = gridsize_S;
	trip_info.gridsize_T = gridsize_T;
	trip_info.n_partitions = nbOfPartitions;
	TriPartitioningInfo4D::writeTrip4DHeader(header, trip_info);

	return trip_info;
}
