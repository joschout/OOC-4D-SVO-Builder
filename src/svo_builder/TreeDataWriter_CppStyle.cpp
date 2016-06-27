#include "TreeDataWriter_CppStyle.h"

TreeDataWriterCppStyle::TreeDataWriterCppStyle(std::string base_filename) :
	position_in_output_file(0)
{
	string filename = base_filename + string(".tree4ddata");
	fstream_data.open(filename, ios::out | ios::in | ios::binary | ios::trunc);
	if (!fstream_data.is_open())
	{
		cout << "error: no file opened with name " << filename << endl;
	}
}

TreeDataWriterCppStyle::~TreeDataWriterCppStyle()
{
	closeFile();
}

void TreeDataWriterCppStyle::closeFile()
{
	if (fstream_data.is_open())
	{
		fstream_data.close();
	}
}

size_t TreeDataWriterCppStyle::writeVoxelData(const VoxelData& voxelData)
{
	if (!fstream_data.is_open())
	{
		std::cout << ".tree4ddata-file is not or incorrectly opened.";
	}

	char voxelDataAsCharArray[VOXELDATA_SIZE];
	std::memcpy(voxelDataAsCharArray, &voxelData.morton, VOXELDATA_SIZE);

	fstream_data.write(voxelDataAsCharArray, VOXELDATA_SIZE);
	position_in_output_file++;
	return position_in_output_file - 1;
}


