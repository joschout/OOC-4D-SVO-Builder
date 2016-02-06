#ifndef MORTON4D_H
#define MORTON4D_H

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

	/*NOTE: sizof(morton) geeft het aantal bytes terug waaruit morton bestaat
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

	/*NOTE: sizof(morton) geeft het aantal bytes terug waaruit morton bestaat
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

#endif
