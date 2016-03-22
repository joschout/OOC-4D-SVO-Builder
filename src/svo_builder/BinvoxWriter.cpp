#include "BinvoxWriter.h"
#include "ReadBinvox.h"
#include "globals.h"


static char FULLVOXEL(1);
static char EMPTYVOXEL(0);

BinvoxWriter::BinvoxWriter(std::string base_filename, vec3 translation_vec, float scale, const size_t gridsize, int timepoint):
	base_filename(base_filename),
	timepoint(timepoint),
	translation_vec(translation_vec),
    gridsize(gridsize),
    start_of_data_section(0),
    end_of_data_section(0)
{
	createNewBinvoxFile();
}

void BinvoxWriter::createNewBinvoxFile()
{
	std::string filename
		= base_filename + string("_t") + val_to_string(timepoint) + string(".binvox");
	outputfile_dense.open(filename, ios::out | ios::in | ios::binary | ios::trunc);
	if(!outputfile_dense.is_open())
	{
		cout << "error: no file opened with name " << filename << endl;
	}
}


string BinvoxWriter::createHeaderString() const
{
	stringstream headerstream;
	headerstream << "#binvox 1" << endl
		<< "dim " << gridsize << " " << gridsize << " " << gridsize << endl
		<< "translate " << (float)translation_vec[0] << " " << (float)translation_vec[1] << " " << (float)translation_vec[2] << endl
		<< "scale " << (float)scale << endl
		<< "data";

	return headerstream.str();
}


void BinvoxWriter::writeHeader_dense()
{
	outputfile_dense << createHeaderString();
	/*
	MERK OP: outputfile_dense.tellp() geeft de huidige positie van de put pointer.
	Na het schrijven van de header, staat de put pointer nu net na de header.
	Dit is de positie waar het eerste karakter van de data moet worden geschreven.
	*/
	start_of_data_section = outputfile_dense.tellp();
}

void BinvoxWriter::writeHeader_sparse(std::ofstream &outputfile_sparse, vec3 translation_vec, float scale) const
{
	/*
	NOTE: IMPORTANT
	After the header of a binvox file,
	after the keyword "data"
	there must be one line feed char (ASCII code 10) BEFORE the bytes of data.
	Otherwise, SHIT WILL BREAK.
	*/
	outputfile_sparse << createHeaderString() << char(10);
}

void BinvoxWriter::initializeEmptyModel_dense()
{
	if(outputfile_dense.is_open())
	{
		outputfile_dense.seekp(start_of_data_section);
		auto nb_of_voxels = gridsize*gridsize*gridsize;
		for (auto index = 0; index < nb_of_voxels; index++)
		{
			writeEmptyVoxel();
		}
		/*
		MERK OP: de put pointer wijst nu net na de data section.
		Op deze positie gaan we nooit moeten schrijven.
		*/

		end_of_data_section = outputfile_dense.tellp();
		outputfile_dense.flush();
	} else
	{
		cout << "Cannot initialize file " << base_filename + string("_t") + val_to_string(timepoint) + string(".binvox")
			<< " with zero's, FILE NOT OPEN." << endl;
	}
}


void BinvoxWriter::writeVoxel_dense(int x, int y, int z)
{
	if(outputfile_dense.is_open())
	{
		int width_x = gridsize * gridsize;
		int width_z = gridsize;
		int position_in_file = x * width_x + z * width_z + y;

		if (static_cast<long>(start_of_data_section) + position_in_file >= end_of_data_section)
		{
			cout << "something went terribly wrong" << endl;
			cout << "while trying to write voxel x:" << x << ", y:" << y << ", z:" << z << endl;
			cout << " calculated position in file: " << position_in_file << endl;
			cout << " position we try to write to: "
				<< static_cast<long>(start_of_data_section) + position_in_file << endl;
			cout << "end position of file: " << end_of_data_section << endl;
		}

		outputfile_dense.seekp(
			static_cast<long>(start_of_data_section)
			+ position_in_file);
		writeFullVoxel();
	}else
	{
		cout << "Cannot write a dense voxel to file " << base_filename + string("_t") + val_to_string(timepoint) + string(".binvox")
			<< " , FILE NOT OPEN." << endl;
	}
}

/*void BinvoxWriter::sparsify()
{
	// open a new file to write 
	std::string filename_sparse
		= base_filename + string("_t") + val_to_string(timepoint) + string("_sparse") + string(".binvox");
	ofstream outputfile_sparse;
	outputfile_sparse.open(filename_sparse);
	if (!outputfile_sparse.is_open())
	{
		cout << "error: no file opened with name " << filename_sparse << endl;
	}

	writeHeader_sparse(outputfile_sparse, translation_vec, scale);


	//position the input reader of the dense file at the beginning of the data section
	outputfile_dense.seekp(start_of_data_section);

	size_t maxAmountOfVoxels = gridsize * gridsize * gridsize;
	char previous_value;
	char current_value;
	unsigned char current_count;

	outputfile_dense >> current_value >> current_count;
	previous_value = current_value;
	int nbOfSameValuesUpUntilNow = 1;
	auto index = 1;

	// for index in [1, maxAmountOfVoxels]
	while( index < maxAmountOfVoxels &&  outputfile_dense.good())
	{
		outputfile_dense >> current_value >> current_count;
		if(current_value == previous_value) //&& nbOfSameValuesUpUntilNow < 255)
		{
			//previous_value = current_value;
			nbOfSameValuesUpUntilNow++;
			if(nbOfSameValuesUpUntilNow == 256)
			{
				outputfile_sparse 
					<< current_value 
					<< static_cast<unsigned char>(255);
				nbOfSameValuesUpUntilNow = 1;
			}
		}else // current_value /= previous_value
		{
			outputfile_sparse
				<< previous_value
				<< static_cast<unsigned char>(nbOfSameValuesUpUntilNow);
			previous_value = current_value;
			nbOfSameValuesUpUntilNow = 1;
		}
		index++;
	}
	outputfile_sparse
		<< current_value
		<< static_cast<unsigned char>(nbOfSameValuesUpUntilNow);
	outputfile_sparse.close();

}*/

void BinvoxWriter::sparsify2()
{

	// open a new file to write 
	std::string filename_sparse
		= base_filename + string("_t") + val_to_string(timepoint) + string("_sparse") + string(".binvox");
	if (verbose)
	{
		cout << "----------------------------" << endl;
		cout << "Sparcifying " << base_filename + string("_t") + val_to_string(timepoint) + string(".binvox") << endl;
		cout << " to " << filename_sparse << endl;

	}
	
	ofstream outputfile_sparse;
	outputfile_sparse.open(filename_sparse);
	if (!outputfile_sparse.is_open())
	{
		cout << "error: no file opened with name " << filename_sparse << endl;
	}



	outputfile_sparse << "#binvox 1" << endl
		<< "dim " << gridsize << " " << gridsize << " " << gridsize << endl
		<< "translate " << (float)translation_vec[0] << " " << (float)translation_vec[1] << " " << (float)translation_vec[2] << endl
		<< "scale " << (float)scale << endl
		<< "data" << "\n"; //<< char(10);



	//writeHeader_sparse(outputfile_sparse, translation_vec, scale);


	//position the input reader of the dense file at the beginning of the data section
	outputfile_dense.seekp(start_of_data_section);

	std::string::size_type position_of_found = 0, position_of_nextfound = 0;
	//std::ostringstream oss;
	string dense_data_string;
	outputfile_dense >> dense_data_string;


	size_t amountOfDenseData = dense_data_string.length();

	size_t maxAmountOfVoxels = gridsize * gridsize * gridsize;
	
	if (verbose) {
		cout << "Checking the length of the dense data." << endl;
		cout << "Length of the dense data string: " << amountOfDenseData << " bytes" << endl;
		cout << "Length should be: " << maxAmountOfVoxels << " bytes" << endl;

		if (maxAmountOfVoxels == amountOfDenseData) {
			cout << "Dense data length is CORRECT." << endl;
		}
		else
		{
			cout << "Dense data length is INCORRECT." << endl;
		}
		cout << endl;
	}
	/*
	string::find_first_not_of

	Zoek in de string DENSE naar het eerste karakter dat niet gelijk is aan DENSE[FOUND],
	i.e. het eerste karakter dat niet gelijk is aan het karakter of positie FOUND in de string DENSE.

	De zoekopdracht bevat enkel karakters vertrekkende van en inclusief de positie FOUND in de string.
		
	RETURNS:
	De positie van het eerste karakter dat niet gelijk is.
	ALS er zo geen karakter kan gevonden worden,
	DAN returnt string::npos.
	*/

	long amount_of_sparse_voxels = 0;

	position_of_nextfound 
		= dense_data_string.find_first_not_of(dense_data_string[position_of_found], position_of_found);
	while (position_of_nextfound != std::string::npos) {
//		oss << nextfound - found;
//		oss << dense[found];
		size_t amount = position_of_nextfound - position_of_found;
		
		outputfile_sparse << dense_data_string[position_of_found]; //write VALUE
		if (amount > 255)
		{
			//cout << "amount is biger than 255" << endl;
			outputfile_sparse << char(255); //write COUNT
			position_of_found = position_of_found + 255;
			amount_of_sparse_voxels += 255;
		}else
		{
			outputfile_sparse << char(amount);//write COUNT
			position_of_found = position_of_nextfound;
			amount_of_sparse_voxels += amount;
		}
		
		position_of_nextfound = dense_data_string.find_first_not_of(dense_data_string[position_of_found], position_of_found);
	}
	//since we must not discard the last characters we add them at the end of the string
	std::string rest(dense_data_string.substr(position_of_found));//last run of characters starts at position found 
	outputfile_sparse << dense_data_string[position_of_found] << char(rest.length()); //write VALUE COUNT
	amount_of_sparse_voxels += rest.length();


	if (verbose) {
		cout << "Checking the voxel data for the sparse file..." << endl;
		if (amount_of_sparse_voxels == amountOfDenseData)
		{
			cout << "The sparse number of voxels is EQUAL to the compressed number of voxels" << endl;
			cout << "Compression is CORRECT." << endl;
		}
		else
		{
			cout << "The sparse number of voxels is NOT EQUAL to the compressed number of voxels" << endl;
			cout << "Compression is NOT CORRECT." << endl;
		}
		cout << endl;
	}
/*	size_t maxAmountOfVoxels = gridsize * gridsize * gridsize;
	char previous_value;
	char current_value;
	unsigned char current_count;

	outputfile_dense >> current_value >> current_count;
	previous_value = current_value;
	int nbOfSameValuesUpUntilNow = 1;
	auto index = 1;

	// for index in [1, maxAmountOfVoxels]
	while (index < maxAmountOfVoxels &&  outputfile_dense.good())
	{
		outputfile_dense >> current_value >> current_count;
		if (current_value == previous_value) //&& nbOfSameValuesUpUntilNow < 255)
		{
			//previous_value = current_value;
			nbOfSameValuesUpUntilNow++;
			if (nbOfSameValuesUpUntilNow == 256)
			{
				outputfile_sparse
					<< current_value
					<< static_cast<unsigned char>(255);
				nbOfSameValuesUpUntilNow = 1;
			}
		}
		else // current_value /= previous_value
		{
			outputfile_sparse
				<< previous_value
				<< static_cast<unsigned char>(nbOfSameValuesUpUntilNow);
			previous_value = current_value;
			nbOfSameValuesUpUntilNow = 1;
		}
		index++;
	}
	outputfile_sparse
		<< current_value
		<< static_cast<unsigned char>(nbOfSameValuesUpUntilNow);*/
	outputfile_sparse.close();

}

void BinvoxWriter::sparsify3()
{

	// open a new file to write 
	std::string filename_sparse
		= base_filename + string("_t") + val_to_string(timepoint) + string("_sparse") + string(".binvox");
	if (verbose)
	{
		cout << "----------------------------" << endl;
		cout << "Sparcifying " << base_filename + string("_t") + val_to_string(timepoint) + string(".binvox") << endl;
		cout << " to " << filename_sparse << endl;

	}

	ofstream outputfile_sparse(filename_sparse.c_str(), ios::out | ios::binary);
	if (!outputfile_sparse.is_open())
	{
		cout << "error: no file opened with name " << filename_sparse << endl;
	}



	outputfile_sparse << "#binvox 1" << endl
		<< "dim " << gridsize << " " << gridsize << " " << gridsize << endl
		//<< "translate " << (float)translation_vec[0] << " " << (float)translation_vec[1] << " " << (float)translation_vec[2] << endl
		//<< "scale " << (float)scale << endl
		<< "data" << "\n"; //<< char(10);



						   //writeHeader_sparse(outputfile_sparse, translation_vec, scale);


						   //position the input reader of the dense file at the beginning of the data section
	outputfile_dense.seekp(start_of_data_section);

	std::string::size_type position_of_found = 0, position_of_nextfound = 0;
	//std::ostringstream oss;
	string dense_data_string;
	outputfile_dense >> dense_data_string;


	size_t amountOfDenseData = dense_data_string.length();

	size_t maxAmountOfVoxels = gridsize * gridsize * gridsize;

	if (verbose) {
		cout << "Checking the length of the dense data." << endl;
		cout << "Length of the dense data string: " << amountOfDenseData << " bytes" << endl;
		cout << "Length should be: " << maxAmountOfVoxels << " bytes" << endl;

		if (maxAmountOfVoxels == amountOfDenseData) {
			cout << "Dense data length is CORRECT." << endl;
		}
		else
		{
			cout << "Dense data length is INCORRECT." << endl;
		}
		cout << endl;
	}


	// Write first voxel
	char currentvalue = dense_data_string[0];
	outputfile_sparse.write((char*)&currentvalue, 1);
	char current_seen = 1;

	// Write BINARY Data
	for (size_t x = 0; x < gridsize; x++) {
		for (size_t z = 0; z < gridsize; z++) {
			for (size_t y = 0; y < gridsize; y++) {
				if (x == 0 && y == 0 && z == 0) {
					continue;
				}
				int width_x = gridsize * gridsize;
				int width_z = gridsize;
				int position_in_file = x * width_x + z * width_z + y;
				char nextvalue = dense_data_string[position_in_file];
				if (nextvalue != currentvalue || current_seen == (char)255) {
					outputfile_sparse.write((char*)&current_seen, 1);
					current_seen = 1;
					currentvalue = nextvalue;
					outputfile_sparse.write((char*)&currentvalue, 1);
				}
				else {
					current_seen++;
				}
			}
		}
	}

	// Write rest
	outputfile_sparse.write((char*)&current_seen, 1);



/*	/*
	string::find_first_not_of

	Zoek in de string DENSE naar het eerste karakter dat niet gelijk is aan DENSE[FOUND],
	i.e. het eerste karakter dat niet gelijk is aan het karakter of positie FOUND in de string DENSE.

	De zoekopdracht bevat enkel karakters vertrekkende van en inclusief de positie FOUND in de string.

	RETURNS:
	De positie van het eerste karakter dat niet gelijk is.
	ALS er zo geen karakter kan gevonden worden,
	DAN returnt string::npos.
	#1#

	long amount_of_sparse_voxels = 0;

	position_of_nextfound
		= dense_data_string.find_first_not_of(dense_data_string[position_of_found], position_of_found);
	while (position_of_nextfound != std::string::npos) {
		//		oss << nextfound - found;
		//		oss << dense[found];
		size_t amount = position_of_nextfound - position_of_found;

		outputfile_sparse << dense_data_string[position_of_found]; //write VALUE
		if (amount > 255)
		{
			//cout << "amount is biger than 255" << endl;
			outputfile_sparse << char(255); //write COUNT
			position_of_found = position_of_found + 255;
			amount_of_sparse_voxels += 255;
		}
		else
		{
			outputfile_sparse << char(amount);//write COUNT
			position_of_found = position_of_nextfound;
			amount_of_sparse_voxels += amount;
		}

		position_of_nextfound = dense_data_string.find_first_not_of(dense_data_string[position_of_found], position_of_found);
	}
	//since we must not discard the last characters we add them at the end of the string
	std::string rest(dense_data_string.substr(position_of_found));//last run of characters starts at position found 
	outputfile_sparse << dense_data_string[position_of_found] << char(rest.length()); //write VALUE COUNT
	amount_of_sparse_voxels += rest.length();


	if (verbose) {
		cout << "Checking the voxel data for the sparse file..." << endl;
		if (amount_of_sparse_voxels == amountOfDenseData)
		{
			cout << "The sparse number of voxels is EQUAL to the compressed number of voxels" << endl;
			cout << "Compression is CORRECT." << endl;
		}
		else
		{
			cout << "The sparse number of voxels is NOT EQUAL to the compressed number of voxels" << endl;
			cout << "Compression is NOT CORRECT." << endl;
		}
		cout << endl;
	}*/
	outputfile_sparse.close();

}
void BinvoxWriter::writeEmptyVoxel()
{
	//unsigned voxel_count(1);
	outputfile_dense << EMPTYVOXEL;// << voxel_count;
}


void BinvoxWriter::writeFullVoxel()
{
	//unsigned voxel_count(1);
	outputfile_dense << FULLVOXEL; //<< voxel_count;
}

void BinvoxWriter::closeFile()
{
	outputfile_dense.close();
}
