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
		do_test();
	}
	
	if (!chip8->load(filename)) 
		return -1;
		
	chip8->run();
	//chip8->decompile();
	
	return 0;
}

void do_test() {
	int h = 12 / 100;
	int t = (12 - (h * 100)) / 10; // why is this coming out to 12?
	int o = 12 - (h * 100) - (t * 10);
	std::cout << h << " " << t << " " << o << std::endl;
}