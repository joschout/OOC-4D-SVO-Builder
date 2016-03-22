#ifndef BINVOXWRITER_H
#define BINVOXWRITER_H
#include <string>
#include <tri_util.h>


class BinvoxWriter
{
public:
	std::string base_filename;
	int timepoint;
	vec3 translation_vec; 
	float scale;
	const size_t gridsize;
	
	std::fstream outputfile_dense;
	std::streampos start_of_data_section;
	std::streampos end_of_data_section;

	BinvoxWriter(std::string base_filename, vec3 translation_vec, float scale, const size_t gridsize, int timepoint);
	void writeHeader_dense();
	void writeHeader_sparse(std::ofstream &outputfile_sparse, vec3 translation_vec, float scale) const;
	void initializeEmptyModel_dense();
	void writeVoxel_dense(int x, int y, int z);
	//void sparsify();
	void sparsify2();
	void sparsify3();
	void closeFile();
private:
	
	void createNewBinvoxFile();
	string createHeaderString() const;
	void writeEmptyVoxel();
	void writeFullVoxel();
};







#endif
