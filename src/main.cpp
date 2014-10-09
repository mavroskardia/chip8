#include "includes.h"

using namespace std;

void do_test();

int main(int argc, char **argv)
{	
	SDL_SetMainReady();

	auto chip8 = new Chip8::Chip8();
	const char* filename = "INVADERS";

	if (argc > 1) {
		filename = argv[1];
	} else {
		cerr << "usage: " << argv[0] << " <romfile>" << endl;
	}

	if (!chip8->load(filename))
		return -1;

	chip8->run();

	return 0;
}


