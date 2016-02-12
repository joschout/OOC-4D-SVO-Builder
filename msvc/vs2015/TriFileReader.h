#ifndef TRIFILEREADER_H
#define TRIFILEREADER_H
#include <string>
#include "../../src/svo_builder/TriInfo4D.h"

TriInfo readTriHeader(std::string& filename, bool verbose);
int parseTri3DHeader(std::string filename, TriInfo& t);
#endif
