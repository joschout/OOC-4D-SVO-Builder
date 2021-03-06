#ifndef TRANSFORMATIONHANDLER_H
#define TRANSFORMATIONHANDLER_H
#include <TriReader.h>

#include "Buffer4D.h"
#include "TriInfo4D.h"

class TransformationHandler
{


public:

	explicit TransformationHandler(size_t gridsize_T);
	const size_t gridsize_T;

	virtual ~TransformationHandler()
	{
	}

	virtual void calculateTransformedBoundingBox(TriInfo4D& triInfo, float end_time) const = 0;

	virtual void transformAndStore(
		const TriInfo4D& tri_info,
		const Triangle& tri, vector<Buffer4D*> &buffers,
		const size_t nbOfPartitions) const = 0;

	void storeTriangleInPartitionBuffers(Triangle4D transformed_tri, vector<Buffer4D*>& buffers, const size_t nbOfPartitions) const;
};
#endif