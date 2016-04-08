#ifndef MORTON4D_H
#define MORTON4D_H
#include <iostream>
#include <bitset>

using std::cout;
using std::endl;

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


//=====================================================//
//=================== UNEQUAL SIDES ===================//
//=====================================================//


// ENCODE 3D 64-bit morton code with possibly unequal gridsides : For loop
template<typename morton, typename coord>
inline morton morton3D_Encode_for(
	const coord x, const coord y, const coord z,
	const size_t gridsize_x, const size_t gridsize_y, const size_t gridsize_z) {
	morton answer = 0;

	/*NOTE: sizeof(morton) geeft het aantal bytes terug waaruit morton bestaat
	DUS: Als morton uit kleiner of gelijk is dan 4 bytes/32 bits
	DAN gebruik 10 bits per coordinaat voor de morton-code
	ELSE gebruik 21 bits per coordinaat voor de morton code
	*/

	//BEWARE: The log of zero is infinity!

	unsigned int checkbits = (sizeof(morton) <= 4) ? 10 : 21;

	/*	int nbOfDimensionsLeft = 3;
	int diff_y = 1;
	int diff_z = 2;*/


	int nbOfBitsLeft_x = log2(gridsize_x);
	int nbOfBitsLeft_y = log2(gridsize_y);
	int nbOfBitsLeft_z = log2(gridsize_z);

	if (gridsize_x == 0)
	{
		nbOfBitsLeft_x = 0;
	}
	if (gridsize_y == 0)
	{
		nbOfBitsLeft_y = 0;
	}
	if (gridsize_z == 0)
	{
		nbOfBitsLeft_z = 0;
	}

	std::cout << "Starting method with" << std::endl;
	std::cout << "x: " << x << ", y: " << y << ", z: " << z << std::endl;
	std::cout << "gridsize X: " << gridsize_x << ", Y: " << gridsize_y << ", Z: " << gridsize_z << std::endl;

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
	}*/

	int current_position_in_morton = 0;

	for (unsigned int index_in_coord = 0; index_in_coord <= checkbits; ++index_in_coord) {
		morton bitmask_index_in_coord = ((morton)0x1 << index_in_coord);
		std::cout << "current bit in coord: " << log2(bitmask_index_in_coord) << std::endl;
		std::cout << "nb of bits left: X: " << nbOfBitsLeft_x << ", Y: " << nbOfBitsLeft_y << ", Z: " << nbOfBitsLeft_z << std::endl;

		if (nbOfBitsLeft_x != 0)
		{
			//take the bit in X;
			morton bitTakenFromX = x & bitmask_index_in_coord;
			cout << "   taken bit " << (bitTakenFromX == 0 ? 0 : 1) << " from x-coord" << std::endl;

			//put the bit on the correct position for the morton code
			morton bitOnCorrectPosition = bitTakenFromX << (current_position_in_morton - index_in_coord);
			cout << "   now using bitmask " << std::bitset<21>(bitOnCorrectPosition) << std::endl;

			answer |= bitOnCorrectPosition;
			std::cout << "   current answer: " << answer << std::endl;
			std::cout << "   current answer in binary: " << std::bitset<21>(answer) << std::endl;
			current_position_in_morton++;
			nbOfBitsLeft_x--;
		}

		if (nbOfBitsLeft_y != 0) {
			//take the bit in Y;
			morton bitTakenFromY = y & bitmask_index_in_coord;
			cout << "   taken bit " << (bitTakenFromY == 0 ? 0 : 1) << " from y-coord" << std::endl;

			//put the bit on the correct position for the morton code
			morton bitOnCorrectPosition = bitTakenFromY << (current_position_in_morton - index_in_coord);
			cout << "   now using bitmask " << std::bitset<21>(bitOnCorrectPosition) << std::endl;

			answer |= bitOnCorrectPosition;
			std::cout << "   current answer: " << answer << std::endl;
			std::cout << "   current answer in binary: " << std::bitset<21>(answer) << std::endl;
			current_position_in_morton++;
			nbOfBitsLeft_y--;
		}

		if (nbOfBitsLeft_z != 0) {
			//take the bit in Z;
			morton bitTakenFromZ = z & bitmask_index_in_coord;
			cout << "   taken bit " << (bitTakenFromZ == 0 ? 0 : 1) << " from z-coord" << std::endl;

			//put the bit on the correct position for the morton code
			morton bitOnCorrectPosition = bitTakenFromZ << (current_position_in_morton - index_in_coord);
			cout << "   now using bitmask " << std::bitset<21>(bitOnCorrectPosition) << std::endl;

			//put the bit in the morton code
			answer |= bitOnCorrectPosition;
			std::cout << "   current answer: " << answer << std::endl;
			std::cout << "   current answer in binary: " << std::bitset<21>(answer) << std::endl;
			current_position_in_morton++;
			nbOfBitsLeft_z--;
		}

		std::cout << "----------" << std::endl;
	}

	std::cout << "===== End of method =====" << std::endl;
	return answer;
}


// DECODE 3D 64-bit morton code code with possibly unequal gridsides: For loop
template<typename morton, typename coord>
inline void morton3D_Decode_for(
	const morton m, coord& x, coord& y, coord& z,
	const size_t gridsize_x, const size_t gridsize_y, const size_t gridsize_z) {
	x = 0;
	y = 0;
	z = 0;

	unsigned int checkbits = (sizeof(morton) <= 4) ? 10 : 21;

	int nbOfBitsLeft_x = log2(gridsize_x);
	int nbOfBitsLeft_y = log2(gridsize_y);
	int nbOfBitsLeft_z = log2(gridsize_z);

	if (gridsize_x == 0)
	{
		nbOfBitsLeft_x = 0;
	}
	if (gridsize_y == 0)
	{
		nbOfBitsLeft_y = 0;
	}
	if (gridsize_z == 0)
	{
		nbOfBitsLeft_z = 0;
	}

	std::cout << "Starting method with" << std::endl;
	std::cout << "morton: " << m << std::endl;
	std::cout << "gridsize X: " << gridsize_x << ", Y: " << gridsize_y << ", Z: " << gridsize_z << std::endl;


	int current_position_in_morton = 0;

	for (unsigned int index_in_coord = 0; index_in_coord <= checkbits; ++index_in_coord) {

		std::cout << "nb of bits left: X: " << nbOfBitsLeft_x << ", Y: " << nbOfBitsLeft_y << ", Z: " << nbOfBitsLeft_z << std::endl;

		if (nbOfBitsLeft_x != 0)
		{
			//take the bit from the morton code;
			morton bitmaskToTakeBitOutOfMorton = 1ull << current_position_in_morton;
			morton bitTakenFromMorton = m & bitmaskToTakeBitOutOfMorton;
			cout << "   taken bit " << (bitTakenFromMorton == 0 ? 0 : 1) << " from morton code" << std::endl;

			//put the bit on the correct position for X coordinate
			morton bitOnPositionForXCoord = bitTakenFromMorton >> (current_position_in_morton - index_in_coord);
			cout << "   now using bitmask " << std::bitset<21>(bitOnPositionForXCoord) << std::endl;

			x |= bitOnPositionForXCoord;
			std::cout << "   current x-coord: " << x << std::endl;
			std::cout << "   current answer in binary: " << std::bitset<21>(x) << std::endl;
			current_position_in_morton++;
			nbOfBitsLeft_x--;
		}

		if (nbOfBitsLeft_y != 0) {
			// take the bit from the morton code;
			morton bitmaskToTakeBitOutOfMorton = 1ull << current_position_in_morton;
			morton bitTakenFromMorton = m & bitmaskToTakeBitOutOfMorton;
			cout << "   taken bit " << (bitTakenFromMorton == 0 ? 0 : 1) << " from morton code" << std::endl;

			//put the bit on the correct position for Y coordinate
			morton bitOnPositionForYCoord = bitTakenFromMorton >> (current_position_in_morton - index_in_coord);
			cout << "   now using bitmask " << std::bitset<21>(bitOnPositionForYCoord) << std::endl;

			y |= bitOnPositionForYCoord;
			std::cout << "   current y-coord: " << y << std::endl;
			std::cout << "   current answer in binary: " << std::bitset<21>(y) << std::endl;
			current_position_in_morton++;
			nbOfBitsLeft_y--;
		}

		if (nbOfBitsLeft_z != 0) {
			// take the bit from the morton code;
			morton bitmaskToTakeBitOutOfMorton = 1ull << current_position_in_morton;
			morton bitTakenFromMorton = m & bitmaskToTakeBitOutOfMorton;
			cout << "   taken bit " << (bitTakenFromMorton == 0 ? 0 : 1) << " from morton code" << std::endl;

			//put the bit on the correct position for Z coordinate
			morton bitOnPositionForZCoord = bitTakenFromMorton >> (current_position_in_morton - index_in_coord);
			cout << "   now using bitmask " << std::bitset<21>(bitOnPositionForZCoord) << std::endl;

			z |= bitOnPositionForZCoord;
			std::cout << "   current z-coord: " << z << std::endl;
			std::cout << "   current answer in binary: " << std::bitset<21>(z) << std::endl;
			current_position_in_morton++;
			nbOfBitsLeft_z--;
		}

		std::cout << "----------" << std::endl;
	}

	std::cout << "===== End of method =====" << std::endl;
}

// ENCODE 4D 64-bit morton code code with possibly unequal gridsides: For loop
template<typename morton, typename coord>
inline morton morton4D_Encode_for(
	const coord x, const coord y, const coord z, const coord t, 
	const size_t gridsize_x, const size_t gridsize_y, const size_t gridsize_z, const size_t gridsize_t) {
	morton answer = 0;

	/*NOTE: sizeof(morton) geeft het aantal bytes terug waaruit morton bestaat
	DUS: Als morton uit kleiner of gelijk is dan 4 bytes/32 bits
	DAN gebruik 10 bits per coordinaat voor de morton-code
	ELSE gebruik 21 bits per coordinaat voor de morton code
	*/

	//BEWARE: The log of zero is infinity!

	unsigned int checkbits = (sizeof(morton) <= 4) ? 8 : 16;

	/*	int nbOfDimensionsLeft = 3;
	int diff_y = 1;
	int diff_z = 2;*/


	int nbOfBitsLeft_x = log2(gridsize_x);
	int nbOfBitsLeft_y = log2(gridsize_y);
	int nbOfBitsLeft_z = log2(gridsize_z);
	int nbOfBitsLeft_t = log2(gridsize_t);

	if (gridsize_x == 0)
	{
		nbOfBitsLeft_x = 0;
	}
	if (gridsize_y == 0)
	{
		nbOfBitsLeft_y = 0;
	}
	if (gridsize_z == 0)
	{
		nbOfBitsLeft_z = 0;
	}
	if (gridsize_t == 0)
	{
		nbOfBitsLeft_t = 0;
	}

	std::cout << "Starting method with" << std::endl;
	std::cout << "x: " << x << ", y: " << y << ", z: " << z << std::endl;
	std::cout << "gridsize X: " << gridsize_x << ", Y: " << gridsize_y << ", Z: " << gridsize_t << ", T: " << gridsize_t << std::endl;

	int current_position_in_morton = 0;

	for (unsigned int index_in_coord = 0; index_in_coord <= checkbits; ++index_in_coord) {
		morton bitmask_index_in_coord = ((morton)0x1 << index_in_coord);
		std::cout << "current bit in coord: " << log2(bitmask_index_in_coord) << std::endl;
		std::cout 
			<< "nb of bits left: X: " << nbOfBitsLeft_x 
			<< ", Y: " << nbOfBitsLeft_y 
			<< ", Z: " << nbOfBitsLeft_z 
			<< ", T: " << nbOfBitsLeft_t << std::endl;

		if (nbOfBitsLeft_x != 0)
		{
			//take the bit in X;
			morton bitTakenFromX = x & bitmask_index_in_coord;
			cout << "   taken bit " << (bitTakenFromX == 0 ? 0 : 1) << " from x-coord" << std::endl;

			//put the bit on the correct position for the morton code
			morton bitOnCorrectPosition = bitTakenFromX << (current_position_in_morton - index_in_coord);
			cout << "   now using bitmask " << std::bitset<21>(bitOnCorrectPosition) << std::endl;

			answer |= bitOnCorrectPosition;
			std::cout << "   current answer: " << answer << std::endl;
			std::cout << "   current answer in binary: " << std::bitset<21>(answer) << std::endl;
			current_position_in_morton++;
			nbOfBitsLeft_x--;
		}

		if (nbOfBitsLeft_y != 0) {
			//take the bit in Y;
			morton bitTakenFromY = y & bitmask_index_in_coord;
			cout << "   taken bit " << (bitTakenFromY == 0 ? 0 : 1) << " from y-coord" << std::endl;

			//put the bit on the correct position for the morton code
			morton bitOnCorrectPosition = bitTakenFromY << (current_position_in_morton - index_in_coord);
			cout << "   now using bitmask " << std::bitset<21>(bitOnCorrectPosition) << std::endl;

			answer |= bitOnCorrectPosition;
			std::cout << "   current answer: " << answer << std::endl;
			std::cout << "   current answer in binary: " << std::bitset<21>(answer) << std::endl;
			current_position_in_morton++;
			nbOfBitsLeft_y--;
		}

		if (nbOfBitsLeft_z != 0) {
			//take the bit in Z;
			morton bitTakenFromZ = z & bitmask_index_in_coord;
			cout << "   taken bit " << (bitTakenFromZ == 0 ? 0 : 1) << " from z-coord" << std::endl;

			//put the bit on the correct position for the morton code
			morton bitOnCorrectPosition = bitTakenFromZ << (current_position_in_morton - index_in_coord);
			cout << "   now using bitmask " << std::bitset<21>(bitOnCorrectPosition) << std::endl;

			//put the bit in the morton code
			answer |= bitOnCorrectPosition;
			std::cout << "   current answer: " << answer << std::endl;
			std::cout << "   current answer in binary: " << std::bitset<21>(answer) << std::endl;
			current_position_in_morton++;
			nbOfBitsLeft_z--;
		}
		if (nbOfBitsLeft_t != 0) {
			//take the bit in T;
			morton bitTakenFromT = t & bitmask_index_in_coord;
			cout << "   taken bit " << (bitTakenFromT == 0 ? 0 : 1) << " from t-coord" << std::endl;

			//put the bit on the correct position for the morton code
			morton bitOnCorrectPosition = bitTakenFromT << (current_position_in_morton - index_in_coord);
			cout << "   now using bitmask " << std::bitset<21>(bitOnCorrectPosition) << std::endl;

			//put the bit in the morton code
			answer |= bitOnCorrectPosition;
			std::cout << "   current answer: " << answer << std::endl;
			std::cout << "   current answer in binary: " << std::bitset<21>(answer) << std::endl;
			current_position_in_morton++;
			nbOfBitsLeft_t--;
		}

		std::cout << "----------" << std::endl;
	}

	std::cout << "===== End of method =====" << std::endl;
	return answer;
}

// DECODE 4D 64-bit morton code code with possibly unequal gridsides: For loop
template<typename morton, typename coord>
inline void morton4D_Decode_for(
	const morton m, coord& x, coord& y, coord& z, coord& t,
	const size_t gridsize_x, const size_t gridsize_y, const size_t gridsize_z, const size_t gridsize_t) {
	x = 0;
	y = 0;
	z = 0;
	t = 0;

	unsigned int checkbits = (sizeof(morton) <= 4) ? 10 : 21;

	int nbOfBitsLeft_x = log2(gridsize_x);
	int nbOfBitsLeft_y = log2(gridsize_y);
	int nbOfBitsLeft_z = log2(gridsize_z);
	int nbOfBitsLeft_t = log2(gridsize_t);

	if (gridsize_x == 0)
	{
		nbOfBitsLeft_x = 0;
	}
	if (gridsize_y == 0)
	{
		nbOfBitsLeft_y = 0;
	}
	if (gridsize_z == 0)
	{
		nbOfBitsLeft_z = 0;
	}
	if(gridsize_t == 0)
	{
		nbOfBitsLeft_t = 0;
	}

	std::cout << "Starting method with" << std::endl;
	std::cout << "morton: " << m << std::endl;
	std::cout 
		<< "gridsize X: " << gridsize_x 
		<< ", Y: " << gridsize_y 
		<< ", Z: " << gridsize_z 
		<< ", T: "<< gridsize_t << std::endl;


	int current_position_in_morton = 0;

	for (unsigned int index_in_coord = 0; index_in_coord <= checkbits; ++index_in_coord) {

		std::cout 
			<< "nb of bits left: X: " << nbOfBitsLeft_x 
			<< ", Y: " << nbOfBitsLeft_y 
			<< ", Z: " << nbOfBitsLeft_z 
			<< ", T: " << nbOfBitsLeft_t << std::endl;

		if (nbOfBitsLeft_x != 0)
		{
			//take the bit from the morton code;
			morton bitmaskToTakeBitOutOfMorton = 1ull << current_position_in_morton;
			morton bitTakenFromMorton = m & bitmaskToTakeBitOutOfMorton;
			cout << "   taken bit " << (bitTakenFromMorton == 0 ? 0 : 1) << " from morton code" << std::endl;

			//put the bit on the correct position for X coordinate
			morton bitOnPositionForXCoord = bitTakenFromMorton >> (current_position_in_morton - index_in_coord);
			cout << "   now using bitmask " << std::bitset<21>(bitOnPositionForXCoord) << std::endl;

			x |= bitOnPositionForXCoord;
			std::cout << "   current x-coord: " << x << std::endl;
			std::cout << "   current answer in binary: " << std::bitset<21>(x) << std::endl;
			current_position_in_morton++;
			nbOfBitsLeft_x--;
		}

		if (nbOfBitsLeft_y != 0) {
			// take the bit from the morton code;
			morton bitmaskToTakeBitOutOfMorton = 1ull << current_position_in_morton;
			morton bitTakenFromMorton = m & bitmaskToTakeBitOutOfMorton;
			cout << "   taken bit " << (bitTakenFromMorton == 0 ? 0 : 1) << " from morton code" << std::endl;

			//put the bit on the correct position for Y coordinate
			morton bitOnPositionForYCoord = bitTakenFromMorton >> (current_position_in_morton - index_in_coord);
			cout << "   now using bitmask " << std::bitset<21>(bitOnPositionForYCoord) << std::endl;

			y |= bitOnPositionForYCoord;
			std::cout << "   current y-coord: " << y << std::endl;
			std::cout << "   current answer in binary: " << std::bitset<21>(y) << std::endl;
			current_position_in_morton++;
			nbOfBitsLeft_y--;
		}

		if (nbOfBitsLeft_z != 0) {
			// take the bit from the morton code;
			morton bitmaskToTakeBitOutOfMorton = 1ull << current_position_in_morton;
			morton bitTakenFromMorton = m & bitmaskToTakeBitOutOfMorton;
			cout << "   taken bit " << (bitTakenFromMorton == 0 ? 0 : 1) << " from morton code" << std::endl;

			//put the bit on the correct position for Z coordinate
			morton bitOnPositionForZCoord = bitTakenFromMorton >> (current_position_in_morton - index_in_coord);
			cout << "   now using bitmask " << std::bitset<21>(bitOnPositionForZCoord) << std::endl;

			z |= bitOnPositionForZCoord;
			std::cout << "   current z-coord: " << z << std::endl;
			std::cout << "   current answer in binary: " << std::bitset<21>(z) << std::endl;
			current_position_in_morton++;
			nbOfBitsLeft_z--;
		}

		if (nbOfBitsLeft_t != 0) {
			// take the bit from the morton code;
			morton bitmaskToTakeBitOutOfMorton = 1ull << current_position_in_morton;
			morton bitTakenFromMorton = m & bitmaskToTakeBitOutOfMorton;
			cout << "   taken bit " << (bitTakenFromMorton == 0 ? 0 : 1) << " from morton code" << std::endl;

			//put the bit on the correct position for T coordinate
			morton bitOnPositionForTCoord = bitTakenFromMorton >> (current_position_in_morton - index_in_coord);
			cout << "   now using bitmask " << std::bitset<21>(bitOnPositionForTCoord) << std::endl;

			t |= bitOnPositionForTCoord;
			std::cout << "   current t-coord: " << t << std::endl;
			std::cout << "   current answer in binary: " << std::bitset<21>(t) << std::endl;
			current_position_in_morton++;
			nbOfBitsLeft_t--;
		}

		std::cout << "----------" << std::endl;
	}

	std::cout << "===== End of method =====" << std::endl;
}

#endif
