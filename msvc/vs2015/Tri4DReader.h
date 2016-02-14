#ifndef TRI4D_READER_H_
#define TRI4D_READER_H_
#include "tri4D_tools.h"
#include <stdio.h>
#include "Triangle4D.h"

using namespace std;
using namespace trimesh;

// A class to read triangles from a .tridata file.
class Tri4DReader {
	size_t n_triangles;
	size_t n_read;
	size_t n_served;

	size_t current_tri; // current triangle id we're going to read

	size_t buffersize;
	Triangle4D* buffer;

	FILE* file;

public:
	Tri4DReader();
	Tri4DReader(const Tri4DReader&);
	Tri4DReader(const std::string &filename, size_t n_triangles, size_t buffersize);
	void getTriangle(Triangle4D& t);
	Triangle4D getTriangle();
	bool hasNext();
	~Tri4DReader();
private:
	void fillBuffer();
};

inline Tri4DReader::Tri4DReader() {
	// TODO
}

inline Tri4DReader::Tri4DReader(const Tri4DReader&) {
	// TODO
}

inline Tri4DReader::Tri4DReader(const std::string &filename, size_t n_triangles, size_t buffersize) : n_triangles(n_triangles), buffersize(buffersize), n_read(0), current_tri(0), n_served(0) {
	// prepare buffer
	buffer = new Triangle4D[buffersize];
	// prepare file
	file = fopen(filename.c_str(), "rb");
	// fill Buffer
	fillBuffer();
}

inline Triangle4D Tri4DReader::getTriangle() {
	if (current_tri == buffersize) { // at end of buffer, refill it
		fillBuffer();
		current_tri = 0;
	}
	Triangle4D t = buffer[current_tri]; // assign triangle from buffer
	current_tri++; // set index for next triangle
	n_served++;
	return t;
}

inline void Tri4DReader::getTriangle(Triangle4D& t) {
	if (current_tri == buffersize) { // at end of buffer, refill it
		fillBuffer();
		current_tri = 0;
	}
	t = buffer[current_tri]; // assign triangle from buffer
	current_tri++; // set index for next triangle
	n_served++;
}

inline bool Tri4DReader::hasNext() {
	return (n_served < n_triangles);
}

inline void Tri4DReader::fillBuffer() {
	size_t readcount = min(buffersize, n_triangles - n_read); // don't read more than there are
	readTriangles4D(file, buffer[0], readcount); // read new triangles
	n_read += readcount; // update the number of tri's we've read
}

inline Tri4DReader::~Tri4DReader() {
	delete buffer;
	fclose(file);
}
#endif