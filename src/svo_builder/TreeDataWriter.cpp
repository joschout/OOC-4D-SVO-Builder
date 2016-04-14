#include "TreeDataWriter.h"

TreeDataWriter::TreeDataWriter(): file_pointer(nullptr), position_in_output_file(0)
{
}

TreeDataWriter::TreeDataWriter(std::string base_filename) : position_in_output_file(0)
{
	string data_name = base_filename + string(".tree4ddata");
	file_pointer = fopen(data_name.c_str(), "wb");
}

TreeDataWriter::~TreeDataWriter()
{
	closeFile();
}

void TreeDataWriter::closeFile()
{
	if (!hasBeenClosed)
	{
		fclose(file_pointer);
	}
}

size_t TreeDataWriter::writeVoxelData(const VoxelData &voxelData)
{
	fwrite(&voxelData.morton, VOXELDATA_SIZE, 1, file_pointer);
	position_in_output_file++;
	return position_in_output_file - 1;
}