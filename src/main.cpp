#include "Chip8.h"
#include <iostream>

void do_test();

int main(int argc, char **argv)
{
	auto chip8 = new Chip8::Chip8();
	const char* filename = "INVADERS";

	if (argc > 1) {
		filename = argv[1];
	} else {
		std::cerr << "usage: " << argv[0] << " <romfile>" << std::endl;
	}

	if (!chip8->load(filename))
		return -1;

	chip8->run();

	return 0;
}
