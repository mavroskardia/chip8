#ifndef __CONVOLVER_H
#define __CONVOLVER_H

#include "includes.h"

class Convolver
{
private:
	Convolver() {};

	float eq(int j, int k);
	void set_pixel_data(int x, int y, SDL_Surface* surface, unsigned int pixel);
	unsigned int get_pixel_data(int x, int y, SDL_Surface* surface);

	int size;
	float* matrix;

public:
	Convolver(int size, float* matrix) : size(size), matrix(matrix) { };
	~Convolver() {
		delete[] matrix;
	};

	unsigned int convolve(unsigned int x, unsigned int y, SDL_Surface* surface);
	void run(SDL_Surface* surface, SDL_Surface* new_surface);
};

#endif // __CONVOLVER_H
