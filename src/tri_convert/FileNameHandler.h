#ifndef	FILENAMEHANDLER_H
#define FILENAMEHANDLER_H
#include <string>


inline int getBaseFileNameWithoutNumbersAtTheEnd(std::string original_filename, std::string& filename_without_numbers_and_extension)
{
	// e.g. translating_box_000001.tri ==> translating_box_000001
	std::string base_filename_with_number = original_filename.substr(0, original_filename.find_last_of("."));

	//count the number of digits at the end of the string
	int numberOfDigitsAtTheEnd = 0;
	while (numberOfDigitsAtTheEnd < base_filename_with_number.length())
	{
		char s_char = base_filename_with_number[base_filename_with_number.length() - 1 - numberOfDigitsAtTheEnd];
		if (!isdigit(s_char)) //if the char is not a number, break
		{
			break;
		}
		numberOfDigitsAtTheEnd++;
	}


	std::string base_filename_without_number = std::string(base_filename_with_number);
	int startingPositionOfNumber = base_filename_with_number.length() - numberOfDigitsAtTheEnd;
	base_filename_without_number.erase(startingPositionOfNumber, numberOfDigitsAtTheEnd);

	filename_without_numbers_and_extension = base_filename_without_number;

	std::string numberAtTheEnd = base_filename_with_number.substr(startingPositionOfNumber, numberOfDigitsAtTheEnd);
	int nbOfCharsInNumberAtTheEnd = numberAtTheEnd.length();

	return nbOfCharsInNumberAtTheEnd;
}




#endif