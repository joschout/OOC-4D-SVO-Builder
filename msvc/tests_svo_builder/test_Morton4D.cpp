#include "../../src/svo_builder/morton4D.h"
#include <gtest/gtest.h>

TEST(Morton3D_Equal_Gridsides, Test3DEncoding1)
{
	//NOTE X runs fastest
	EXPECT_EQ(0, morton3D_Encode_for<uint64_t>(0, 0, 0));
	EXPECT_EQ(1, morton3D_Encode_for<uint64_t>(1, 0, 0));
	EXPECT_EQ(2, morton3D_Encode_for<uint64_t>(0, 1, 0));
	EXPECT_EQ(3, morton3D_Encode_for<uint64_t>(1, 1, 0));
	EXPECT_EQ(4, morton3D_Encode_for<uint64_t>(0, 0, 1));
	EXPECT_EQ(5, morton3D_Encode_for<uint64_t>(1, 0, 1));
	EXPECT_EQ(6, morton3D_Encode_for<uint64_t>(0, 1, 1));
	EXPECT_EQ(7, morton3D_Encode_for<uint64_t>(1, 1, 1));
}
TEST(Morton3D_Equal_Gridsides, Test3DDecoding1)
{
	//NOTE X runs fastest
	
	unsigned int x, y, z;
	uint64_t morton = 0;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z);
	EXPECT_EQ(0, x);
	EXPECT_EQ(0, y);
	EXPECT_EQ(0, z);

	morton = 1;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z);
	EXPECT_EQ(1, x);
	EXPECT_EQ(0, y);
	EXPECT_EQ(0, z);


	morton = 2;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z);
	EXPECT_EQ(0, x);
	EXPECT_EQ(1, y);
	EXPECT_EQ(0, z);

	morton = 3;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z);
	EXPECT_EQ(1, x);
	EXPECT_EQ(1, y);
	EXPECT_EQ(0, z);

	morton = 4;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z);
	EXPECT_EQ(0, x);
	EXPECT_EQ(0, y);
	EXPECT_EQ(1, z);

	morton = 5;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z);
	EXPECT_EQ(1, x);
	EXPECT_EQ(0, y);
	EXPECT_EQ(1, z);

	morton = 6;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z);
	EXPECT_EQ(0, x);
	EXPECT_EQ(1, y);
	EXPECT_EQ(1, z);

	morton = 7;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z);
	EXPECT_EQ(1, x);
	EXPECT_EQ(1, y);
	EXPECT_EQ(1, z);
}

TEST(Morton3D_Different_Gridsides, TestEncoding_EqualGridSides1)
{
	EXPECT_EQ(0, morton3D_Encode_for<uint64_t>(0, 0, 0, 2, 2 ,2));
	EXPECT_EQ(1, morton3D_Encode_for<uint64_t>(1, 0, 0, 2, 2, 2));
	EXPECT_EQ(2, morton3D_Encode_for<uint64_t>(0, 1, 0, 2, 2, 2));
	EXPECT_EQ(3, morton3D_Encode_for<uint64_t>(1, 1, 0, 2, 2, 2));
	EXPECT_EQ(4, morton3D_Encode_for<uint64_t>(0, 0, 1, 2, 2, 2));
	EXPECT_EQ(5, morton3D_Encode_for<uint64_t>(1, 0, 1, 2, 2, 2));
	EXPECT_EQ(6, morton3D_Encode_for<uint64_t>(0, 1, 1, 2, 2, 2));
	EXPECT_EQ(7, morton3D_Encode_for<uint64_t>(1, 1, 1, 2, 2, 2));
}
TEST(Morton3D_Different_Gridsides, TestEncoding_UNEqualGridSides2)
{
	//Gridsizes:  Z = 0, Y = 8, X = 2
	//											X, Y, Z, SX,SY,SZ
	EXPECT_EQ(0,  morton3D_Encode_for<uint64_t>(0, 0, 0, 2, 8, 0));
	EXPECT_EQ(1,  morton3D_Encode_for<uint64_t>(1, 0, 0, 2, 8, 0));
	EXPECT_EQ(2,  morton3D_Encode_for<uint64_t>(0, 1, 0, 2, 8, 0));
	EXPECT_EQ(3,  morton3D_Encode_for<uint64_t>(1, 1, 0, 2, 8, 0));
	EXPECT_EQ(4,  morton3D_Encode_for<uint64_t>(0, 2, 0, 2, 8, 0));
	EXPECT_EQ(5,  morton3D_Encode_for<uint64_t>(1, 2, 0, 2, 8, 0));
	EXPECT_EQ(6,  morton3D_Encode_for<uint64_t>(0, 3, 0, 2, 8, 0));
	EXPECT_EQ(7,  morton3D_Encode_for<uint64_t>(1, 3, 0, 2, 8, 0));
	//vanaf hier loopt het nog mis
	EXPECT_EQ(8,  morton3D_Encode_for<uint64_t>(0, 4, 0, 2, 8, 0));
	EXPECT_EQ(9,  morton3D_Encode_for<uint64_t>(1, 4, 0, 2, 8, 0));
	EXPECT_EQ(10, morton3D_Encode_for<uint64_t>(0, 5, 0, 2, 8, 0));
	EXPECT_EQ(11, morton3D_Encode_for<uint64_t>(1, 5, 0, 2, 8, 0));
	EXPECT_EQ(12, morton3D_Encode_for<uint64_t>(0, 6, 0, 2, 8, 0));
	EXPECT_EQ(13, morton3D_Encode_for<uint64_t>(1, 6, 0, 2, 8, 0));
	EXPECT_EQ(14, morton3D_Encode_for<uint64_t>(0, 7, 0, 2, 8, 0));
	EXPECT_EQ(15, morton3D_Encode_for<uint64_t>(1, 7, 0, 2, 8, 0));
}

TEST(Morton3D_Different_Gridsides, TestDecoding_EqualGridSides1)
{
	//NOTE X runs fastest

	unsigned int x, y, z;
	uint64_t morton = 0;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z, 2, 2, 2);
	EXPECT_EQ(0, x);
	EXPECT_EQ(0, y);
	EXPECT_EQ(0, z);

	morton = 1;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z, 2, 2, 2);
	EXPECT_EQ(1, x);
	EXPECT_EQ(0, y);
	EXPECT_EQ(0, z);


	morton = 2;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z, 2, 2, 2);
	EXPECT_EQ(0, x);
	EXPECT_EQ(1, y);
	EXPECT_EQ(0, z);

	morton = 3;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z, 2, 2, 2);
	EXPECT_EQ(1, x);
	EXPECT_EQ(1, y);
	EXPECT_EQ(0, z);

	morton = 4;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z, 2, 2, 2);
	EXPECT_EQ(0, x);
	EXPECT_EQ(0, y);
	EXPECT_EQ(1, z);

	morton = 5;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z, 2, 2, 2);
	EXPECT_EQ(1, x);
	EXPECT_EQ(0, y);
	EXPECT_EQ(1, z);

	morton = 6;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z, 2, 2, 2);
	EXPECT_EQ(0, x);
	EXPECT_EQ(1, y);
	EXPECT_EQ(1, z);

	morton = 7;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z, 2, 2, 2);
	EXPECT_EQ(1, x);
	EXPECT_EQ(1, y);
	EXPECT_EQ(1, z);
}
TEST(Morton3D_Different_Gridsides, TestDecoding_UNEqualGridSides1)
{
	//NOTE X runs fastest

	unsigned int x, y, z;
	uint64_t morton = 0;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z, 2, 8, 0);
	EXPECT_EQ(0, x); EXPECT_EQ(0, y); EXPECT_EQ(0, z);

	morton = 1;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z, 2, 8, 0);
	EXPECT_EQ(1, x); EXPECT_EQ(0, y); EXPECT_EQ(0, z);

	morton = 2;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z, 2, 8, 0);
	EXPECT_EQ(0, x); EXPECT_EQ(1, y); EXPECT_EQ(0, z);

	morton = 3;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z, 2, 8, 0);
	EXPECT_EQ(1, x); EXPECT_EQ(1, y); EXPECT_EQ(0, z);

	morton = 4;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z, 2, 8, 0);
	EXPECT_EQ(0, x); EXPECT_EQ(2, y); EXPECT_EQ(0, z);

	morton = 5;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z, 2, 8, 0);
	EXPECT_EQ(1, x); EXPECT_EQ(2, y); EXPECT_EQ(0, z);

	morton = 6;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z, 2, 8, 0);
	EXPECT_EQ(0, x); EXPECT_EQ(3, y); EXPECT_EQ(0, z);

	morton = 7;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z, 2, 8, 0);
	EXPECT_EQ(1, x); EXPECT_EQ(3, y); EXPECT_EQ(0, z);

	morton = 8;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z, 2, 8, 0);
	EXPECT_EQ(0, x); EXPECT_EQ(4, y); EXPECT_EQ(0, z);

	morton = 9;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z, 2, 8, 0);
	EXPECT_EQ(1, x); EXPECT_EQ(4, y); EXPECT_EQ(0, z);

	morton = 10;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z, 2, 8, 0);
	EXPECT_EQ(0, x); EXPECT_EQ(5, y); EXPECT_EQ(0, z);

	morton = 11;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z, 2, 8, 0);
	EXPECT_EQ(1, x); EXPECT_EQ(5, y); EXPECT_EQ(0, z);

	morton = 12;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z, 2, 8, 0);
	EXPECT_EQ(0, x); EXPECT_EQ(6, y); EXPECT_EQ(0, z);

	morton = 13;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z, 2, 8, 0);
	EXPECT_EQ(1, x); EXPECT_EQ(6, y); EXPECT_EQ(0, z);

	morton = 14;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z, 2, 8, 0);
	EXPECT_EQ(0, x); EXPECT_EQ(7, y); EXPECT_EQ(0, z);

	morton = 15;
	morton3D_Decode_for<uint64_t, unsigned int>(morton, x, y, z, 2, 8, 0);
	EXPECT_EQ(1, x); EXPECT_EQ(7, y); EXPECT_EQ(0, z);
}

TEST(Morton4D_Different_Gridsides, TestEncoding_EqualGridSides1)
{
	EXPECT_EQ(0,  morton4D_Encode_for<uint64_t>(0, 0, 0, 0, 2, 2, 2, 2));
	EXPECT_EQ(1,  morton4D_Encode_for<uint64_t>(1, 0, 0, 0, 2, 2, 2, 2));
	EXPECT_EQ(2,  morton4D_Encode_for<uint64_t>(0, 1, 0, 0, 2, 2, 2, 2));
	EXPECT_EQ(3,  morton4D_Encode_for<uint64_t>(1, 1, 0, 0, 2, 2, 2, 2));
	EXPECT_EQ(4,  morton4D_Encode_for<uint64_t>(0, 0, 1, 0, 2, 2, 2, 2));
	EXPECT_EQ(5,  morton4D_Encode_for<uint64_t>(1, 0, 1, 0, 2, 2, 2, 2));
	EXPECT_EQ(6,  morton4D_Encode_for<uint64_t>(0, 1, 1, 0, 2, 2, 2, 2));
	EXPECT_EQ(7,  morton4D_Encode_for<uint64_t>(1, 1, 1, 0, 2, 2, 2, 2));
	EXPECT_EQ(8,  morton4D_Encode_for<uint64_t>(0, 0, 0, 1, 2, 2, 2, 2));
	EXPECT_EQ(9,  morton4D_Encode_for<uint64_t>(1, 0, 0, 1, 2, 2, 2, 2));
	EXPECT_EQ(10, morton4D_Encode_for<uint64_t>(0, 1, 0, 1, 2, 2, 2, 2));
	EXPECT_EQ(11, morton4D_Encode_for<uint64_t>(1, 1, 0, 1, 2, 2, 2, 2));
	EXPECT_EQ(12, morton4D_Encode_for<uint64_t>(0, 0, 1, 1, 2, 2, 2, 2));
	EXPECT_EQ(13, morton4D_Encode_for<uint64_t>(1, 0, 1, 1, 2, 2, 2, 2));
	EXPECT_EQ(14, morton4D_Encode_for<uint64_t>(0, 1, 1, 1, 2, 2, 2, 2));
	EXPECT_EQ(15, morton4D_Encode_for<uint64_t>(1, 1, 1, 1, 2, 2, 2, 2));
}