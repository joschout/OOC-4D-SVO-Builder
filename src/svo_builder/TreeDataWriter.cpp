//#include "TreeDataWriter.h"
//
//TreeDataWriter::TreeDataWriter(): file_pointer_data(nullptr), position_in_output_file(0), hasBeenClosed(false)
//{
//}
//
//TreeDataWriter::TreeDataWriter(std::string base_filename) : position_in_output_file(0), hasBeenClosed(false)
//{
//	string data_name = base_filename + string(".tree4ddata");
//	std::cout << "Name of file to open: " << data_name << std::endl;
//	file_pointer_data = fopen(data_name.c_str(), "wb");
//	std::cout << "errno: " << strerror(errno) << std::endl;
//
//	if(file_pointer_data == nullptr)
//	{
//		std::cout << ".tree4ddata-file is not or incorrectly opened." << std::endl;
//	}
//	std::cout << "file pointer " << file_pointer_data << std::endl;
//	size_t test = 1000;
//	std::cout << "test size_t: " << test << std::endl;
//	std::cout << "size of size_t: " << sizeof(size_t) << std::endl;
//}
//
//TreeDataWriter::~TreeDataWriter()
//{
//	closeFile();
//}
//
//void TreeDataWriter::closeFile()
//{
//	if (!hasBeenClosed)
//	{
//		fclose(file_pointer_data);
//	}
//}
//
//size_t TreeDataWriter::writeVoxelData(const VoxelData &voxelData)
//{
//	fwrite(&voxelData.morton, VOXELDATA_SIZE, 1, file_pointer_data);
//	position_in_output_file++;
//	return position_in_output_file - 1;
//}
