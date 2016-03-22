#include "BinvoxHandler.h"
#include "globals.h"

BinvoxHandler::BinvoxHandler(std::string base_filename, size_t gridsize):
	base_filename(base_filename), gridsize(gridsize)
{
}

void BinvoxHandler::initialize(vec3 translation_vec, float scale)
{
	binvox_writers.reserve(gridsize);
	for (auto i = 0; i < gridsize; i++)
	{
		binvox_writers.push_back(BinvoxWriter(base_filename, translation_vec, scale, gridsize, i ));
	}
}

BinvoxHandler::BinvoxHandler(): gridsize(0)
{
}

void BinvoxHandler::createInitialBinvoxFiles()
{
	for (std::vector<BinvoxWriter>::iterator it = binvox_writers.begin(); it != binvox_writers.end(); ++it)
	{
		it->writeHeader_dense();
		it->initializeEmptyModel_dense();

	}
}

void BinvoxHandler::writeHeaders()
{
	for (std::vector<BinvoxWriter>::iterator it = binvox_writers.begin(); it != binvox_writers.end(); ++it)
	{
		it->writeHeader_dense();
	}
}

void BinvoxHandler::writeVoxel(int timepoint, int x, int y, int z)
{
	binvox_writers[timepoint].writeVoxel_dense(x, y, z);
}

void BinvoxHandler::sparsifyFiles()
{
	if(verbose)
	{
		cout << "Starting sparcification of binvox files..." << endl;
	}
	for (std::vector<BinvoxWriter>::iterator it = binvox_writers.begin(); it != binvox_writers.end(); ++it)
	{
		it->sparsify3();
	}
}

void BinvoxHandler::closeWriters()
{
	for (std::vector<BinvoxWriter>::iterator it = binvox_writers.begin(); it != binvox_writers.end(); ++it)
	{
		it->closeFile();
	}
}
