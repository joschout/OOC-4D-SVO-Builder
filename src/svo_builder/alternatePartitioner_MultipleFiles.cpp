#include "alternatePartitioner_MultipleFiles.h"
#include "TriPartitioningInfo4D.h"
#include <memory>
#include "PrintStatusBar.h"

alternatePartitioner_MultipleFiles::alternatePartitioner_MultipleFiles():
	alternatePartitioner_Interface(1, 1, 4)
{
}

alternatePartitioner_MultipleFiles::alternatePartitioner_MultipleFiles(size_t gridsize_S, size_t gridsize_T, size_t nbOfDimensions):
	alternatePartitioner_Interface(gridsize_S, gridsize_T, nbOfDimensions)
{
}

TriPartitioningInfo4D alternatePartitioner_MultipleFiles::partitionTriangleModel_multiple_files(TriInfo4D_multiple_files& extended_tri_info, size_t voxel_memory_limit)
{
	//estimate the amount of triangle partitions needed for voxelization
	//NOTE: the estimation is hardcoded for 3D trees
	size_t nbOfTrianglePartitions = estimateNumberOfPartitions(voxel_memory_limit);
	cout << "Partitioning data into " << nbOfTrianglePartitions << " partitions ... " << endl; cout.flush();

	if (data_out)
	{
		partitioning_total_timer.stop();	// TIMING
		data_writer_ptr->writeToFile_endl("number Of partitions: " + to_string(nbOfTrianglePartitions));
		partitioning_total_timer.start();	// TIMING
	}

	//partition the  triangle mesh
	TriPartitioningInfo4D trianglePartition_info = partition_multiple_files(extended_tri_info);
	cout << "done." << endl;

	return trianglePartition_info;
}

// Partition the mesh referenced by tri_info into n triangle partitions for gridsize,
// and store information about the partitioning in trip_info
TriPartitioningInfo4D alternatePartitioner_MultipleFiles::partition_multiple_files(const TriInfo4D_multiple_files& tri_info)
{
	// MAKE ONE TRIDATAREADER PER TRIDATA FILE --> one triReader per moment in time;


	partitioning_io_input_timer.start();	// TIMING

	vector<unique_ptr<TriReader>> tridataReaders_;
	for (const TriInfo& tri_info_3d : tri_info.getTriInfoVector())
	{
		//		cout << "tri_info_3d.base_filename: " << tri_info_3d.base_filename << endl;

		// Open tri_data stream
		// the reader knows how many triangles there are in the model,
		// the reader is given input_buffersize as buffersize for buffering read triangles

		const std::string tridata_filename = tri_info_3d.base_filename + string(".tridata");
		unique_ptr<TriReader>  tridataReader_prt(new TriReader(tridata_filename, tri_info_3d.n_triangles, input_buffersize));
		tridataReaders_.push_back(std::move(tridataReader_prt));
		//		TriReader tridataReader = TriReader(tridata_filename, tri_info_3d.n_triangles, input_buffersize);
		//		tridataReaders.push_back(tridataReader);

		//		cout << "size of tridataReaders vector" << tridataReaders_.size() << endl;
	}

	partitioning_io_input_timer.stop();		// TIMING

	partitioning_algorithm_timer.start();	// TIMING

	// Create Mortonbuffers: we will have one buffer per partition
	vector<Buffer4D*> buffers;
	createBuffers(tri_info.getTotalBoundingBox(), tri_info.base_filename_without_number, buffers);

	//for each moment in time = for each triReader
	float& end_time = tri_info.getTotalBoundingBox().max[3];
	float& start_time = tri_info.getTotalBoundingBox().min[3];
	float unitlength_time = (end_time - start_time) / (float)gridsize_T;
	float time = 0;
	size_t max_amount = tri_info.getNbfOfTriangles();
	size_t current_amount_of_triangles_partitioned = 0;

	cout << "Partitioning triangles: " << endl;
	for (int reader_index = 0; reader_index < tridataReaders_.size(); reader_index++)
	{
		//TriReader& tridataReader = tridataReaders_.at(reader_index);
		//for each triangle
		while (tridataReaders_.at(reader_index)->hasNext()) {
			Triangle tri;

			partitioning_algorithm_timer.stop();	// TIMING
			partitioning_io_input_timer.start();	// TIMING
			tridataReaders_.at(reader_index)->getTriangle(tri);
			partitioning_io_input_timer.stop();		// TIMING
			partitioning_algorithm_timer.start();	// TIMING


			Triangle4D tri_4d = Triangle4D(tri, time);
			storeTriangleInPartitionBuffers(tri_4d, buffers, nbOfPartitions);
			current_amount_of_triangles_partitioned++;
			if(verbose)
			{
				showProgressBar(current_amount_of_triangles_partitioned, max_amount);				
			}
		}

		time += unitlength_time;


		//		cout << "reader_index: " << reader_index << endl;
	}

	partitioning_algorithm_timer.stop();	// TIMING

	partitioning_io_output_timer.start();	// TIMING
	TriPartitioningInfo4D trip_info = createTriPartitioningInfoHeader_multiple_files(tri_info, buffers);
	partitioning_io_output_timer.stop();	// TIMING
	
	deleteBuffers(buffers);

	return trip_info;
}

TriPartitioningInfo4D alternatePartitioner_MultipleFiles::createTriPartitioningInfoHeader_multiple_files(const TriInfo4D_multiple_files& tri_info, vector<Buffer4D*>& buffers) const
{
	TriPartitioningInfo4D trip_info = TriPartitioningInfo4D(tri_info);

	// Collect nbOfTriangles of each partition
	trip_info.nbOfTrianglesPerPartition.resize(nbOfPartitions);
	for (size_t j = 0; j < nbOfPartitions; j++) {
		trip_info.nbOfTrianglesPerPartition[j] = buffers[j]->n_triangles;
	}

	// Write trip header
	trip_info.base_filename
		= tri_info.base_filename_without_number
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
