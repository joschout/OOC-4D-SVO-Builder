#ifndef PRINTUTILS_H
#define PRINTUTILS_H
#include <string>
#include <iostream>

int printCurrentDirectory();
void printInfo(std::string &version);
void printHelp();
void printInvalid();

template <typename T>
void printBuffer(T current_amount, T total_amount)
{
	if (current_amount % (total_amount / 100) != 0) return;

	float progress = (float)current_amount / (float)total_amount;
	int barWidth = 70;

	std::cout << '\r' << "[";
	int pos = barWidth * progress;
	for (int i = 0; i < barWidth; ++i) {
		if (i < pos) std::cout << "=";
		else if (i == pos) std::cout << ">";
		else std::cout << " ";
	}
	std::cout << "] " << int(progress * 100.0) << " %\r";
	std::cout.flush();
}
#endif

