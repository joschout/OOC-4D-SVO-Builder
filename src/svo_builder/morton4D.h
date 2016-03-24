#ifndef MORTON4D_H
#define MORTON4D_H

#include <gtest/gtest.h>
// ENCODE 3D 64-bit morton code : For loop
template<typename morton, typename coord>
inline morton morton3D_Encode_for(const coord x, const coord y, const coord z) {
	morton answer = 0;

	/*NOTE: sizof(morton) geeft het aantal bytes terug waaruit morton bestaat
	DUS: Als morton uit kleiner of gelijk is dan 4 bytes/32 bits
		DAN gebruik 10 bits per coordinaat voor de morton-code
		ELSE gebruik 21 bits per coordinaat voor de morton code
	*/

	unsigned int checkbits = (sizeof(morton) <= 4) ? 10 : 21;
	for (unsigned int i = 0; i <= checkbits; ++i) {
		/*
		take the i'th bit of x
		--> Returns an int full of 0's, with the i'th bit of x on the i'th position.
		--> In the morton code, the ith bit of x is placed on position i + (2*i)
		*/
		answer |= ((x & ((morton)0x1 << i)) << 2 * i)     //Here we need to cast 0x1 to the amount of bits in the morton code, 
			| ((y & ((morton)0x1 << i)) << ((2 * i) + 1))   //otherwise there is a bug when morton code is larger than 32 bits
			| ((z & ((morton)0x1 << i)) << ((2 * i) + 2));
	}
	return answer;
}

// DECODE 3D 64-bit morton code : For loop
template<typename morton, typename coord>
inline void morton3D_Decode_for(const morton m, coord& x, coord& y, coord& z) {
	x = 0; y = 0; z = 0;

	/*NOTE: sizeof(morton) geeft het aantal bytes terug waaruit morton bestaat
	DUS: Als morton uit kleiner of gelijk is dan 4 bytes/32 bits
	DAN gebruik 10 bits per coordinaat voor de morton-code
	ELSE gebruik 21 bits per coordinaat voor de morton code
	*/
	unsigned int checkbits = (sizeof(morton) <= 4) ? 10 : 21;

	for (morton i = 0; i <= checkbits; ++i) {
		x |= (m & (1ull << 3 * i)) >> (2 * i);
		y |= (m & (1ull << ((3 * i) + 1))) >> ((2 * i) + 1);
		z |= (m & (1ull << ((3 * i) + 2))) >> ((2 * i) + 2);
	}
}

// ENCODE 4D 64-bit morton code : For loop
template<typename morton, typename coord>
inline morton morton4D_Encode_for(const coord x, const coord y, const coord z, const coord t) {
	morton answer = 0;

	/*NOTE: sizeof(morton) geeft het aantal bytes terug waaruit morton bestaat
	DUS: Als morton uit kleiner of gelijk is dan 4 bytes/32 bits
	DAN gebruik 8 bits per coordinaat voor de morton-code
	ELSE gebruik 16 bits voor de morton code
	*/
	unsigned int checkbits = (sizeof(morton) <= 4) ? 8 : 16;
	for (unsigned int i = 0; i <= checkbits; ++i) {
		/*
		take the i'th bit of x
		--> Returns an int full of 0's, with the i'th bit of x on the i'th position.
		--> In the morton code, the ith bit of x is placed on position i + (3*i)
		*/
		answer |= ((x & ((morton)0x1 << i)) << 3 * i)     //Here we need to cast 0x1 to the amount of bits in the morton code, 
			| ((y & ((morton)0x1 << i)) << ((3 * i) + 1))   //otherwise there is a bug when morton code is larger than 32 bits
			| ((z & ((morton)0x1 << i)) << ((3 * i) + 2))
			| ((t & ((morton)0x1 << i)) << ((3 * i) + 3));
	}
	return answer;
}

// DECODE 4D 64-bit morton code : For loop
template<typename morton, typename coord>
inline void morton4D_Decode_for(const morton m, coord& x, coord& y, coord& z, coord& t) {
	x = 0; y = 0; z = 0; t = 0;
	unsigned int checkbits = (sizeof(morton) <= 4) ? 8 : 16;

	for (morton i = 0; i <= checkbits; ++i) {
		x |= (m & (1ull << 4 * i)) >> (3 * i);
		y |= (m & (1ull << ((4 * i) + 1))) >> ((3 * i) + 1);
		z |= (m & (1ull << ((4 * i) + 2))) >> ((3 * i) + 2);
		t |= (m & (1ull << ((4 * i) + 3))) >> ((3 * i) + 3);
	}
}



//=================== UNEQUAL SIDES =======//

/*TEST(mortonEncode3DGridsize, firstTest)
{
	
}

// ENCODE 3D 64-bit morton code : For loop
template<typename morton, typename coord>
inline morton morton3D_Encode_for(
	const coord x, const coord y, const coord z,
	const size_t gridsize_x, const size_t gridsize_y, const size_t gridsize_z) {
	morton answer = 0;

	/*NOTE: sizeof(morton) geeft het aantal bytes terug waaruit morton bestaat
	DUS: Als morton uit kleiner of gelijk is dan 4 bytes/32 bits
	DAN gebruik 10 bits per coordinaat voor de morton-code
	ELSE gebruik 21 bits per coordinaat voor de morton code
	#1#

	int nbOfBitsLeft_x = gridsize_x;
	int nbOfBitsLeft_y = gridsize_y;
	int nbOfBitsLeft_z = gridsize_z;
	unsigned int checkbits = (sizeof(morton) <= 4) ? 10 : 21;

	int nbOfDimensionsLeft = 3;
	int diff_y = 1;
	int diff_z = 2;
/*	for (unsigned int index_in_coord = 0; index_in_coord <= checkbits; ++index_in_coord) {
		 if (nbOfBitsLeft_x != 0){
			 int shift = ints_in_between*
			answer |= ((x & ((morton)0x1 << index_in_coord)) << 2 * index_in_coord);
			nbOfBitsLeft_x--;
			if (nbOfBitsLeft_x == 0)
			{
				diff_y--;
				diff_z--;
			}
		}
		else if (nbOfBitsLeft_y != 0){
			answer |= ((y & ((morton)0x1 << index_in_coord)) << ((2 * index_in_coord) + diff_y));
			nbOfBitsLeft_y--;
			if(nbOfBitsLeft_y == 0)
			{
				diff_z--;
			}
		}
		if(nbOfBitsLeft_z != 0){
			answer != ((z & ((morton)0x1 << index_in_coord)) << ((2 * index_in_coord) + diff_z));
			nbOfBitsLeft_z--;
		}
	}#1#
	int current_shift = 0;
	for (unsigned int index_in_coord = 0; index_in_coord <= checkbits; ++index_in_coord) {
		morton bitmask_index_in_coord = ((morton)0x1 << index_in_coord);

		if (nbOfBitsLeft_z != 0) {
			answer != ((z &bitmask_index_in_coord) << (current_shift + diff_z));
			nbOfBitsLeft_z--;
			if(nbOfBitsLeft_z == 0)
			{
				nbOfDimensionsLeft--;
			}
		}
		if (nbOfBitsLeft_y != 0) {
			answer |= ((y & bitmask_index_in_coord) << (current_shift + diff_y));
			nbOfBitsLeft_y--;
			if(nbOfBitsLeft_y == 0)
			{
				diff_z--;
				nbOfDimensionsLeft--;
			}
		}
		if (nbOfBitsLeft_x != 0) {
			answer |= ((x & bitmask_index_in_coord) << current_shift);
			nbOfBitsLeft_x--;
			if(nbOfBitsLeft_x == 0)
			{
				diff_y--;
				diff_z--;
				nbOfDimensionsLeft--;
			}
		}
		current_shift += nbOfDimensionsLeft;
	}
	
	return answer;
}

// DECODE 3D 64-bit morton code : For loop
template<typename morton, typename coord>
inline void morton3D_Decode_for(
	const morton m, coord& x, coord& y, coord& z,
	const size_t gridsize_x, const size_t gridsize_y, const size_t gridsize_z) {
	x = 0; y = 0; z = 0;

	/*NOTE: sizeof(morton) geeft het aantal bytes terug waaruit morton bestaat
	DUS: Als morton uit kleiner of gelijk is dan 4 bytes/32 bits
	DAN gebruik 10 bits per coordinaat voor de morton-code
	ELSE gebruik 21 bits per coordinaat voor de morton code
	#1#
	unsigned int checkbits = (sizeof(morton) <= 4) ? 10 : 21;

	int nbOfBitsLeft_x = gridsize_x;
	int nbOfBitsLeft_y = gridsize_y;
	int nbOfBitsLeft_z = gridsize_z;

	int diff_y = 1;
	int diff_z = 2;

	for (morton i = 0; i <= checkbits; ++i) {
		x |= (m & (1ull << 3 * i)) >> (2 * i);
		y |= (m & (1ull << ((3 * i) + 1))) >> ((2 * i) + 1);
		z |= (m & (1ull << ((3 * i) + 2))) >> ((2 * i) + 2);
	}
}*/
#endif
