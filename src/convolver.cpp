#include "includes.h"

unsigned int Convolver::get_pixel_data(int x, int y, SDL_Surface* surface)
{
	if (x < 0) x = 0;
	if (y < 0) y = 0;

	if (x >= surface->w) x = surface->w - 1;
	if (y >= surface->h) y = surface->h - 1;

	return ((unsigned int*) surface->pixels)[x + y * surface->w];
}

inline void Convolver::set_pixel_data(int x, int y, SDL_Surface* surface, unsigned int pixel)
{
	unsigned int* pixels = (unsigned int*) surface->pixels;
	pixels[x + y * surface->w] = pixel;
}

unsigned int Convolver::convolve(unsigned int x, unsigned int y, SDL_Surface* surface)
{
	float acc = 0.0f, pixel = 0.0f;

	for (int k = -1; k <= 1; k++) {
		for (int j = -1; j <= 1; j++) {
			pixel = ((unsigned int*)surface->pixels)[(x+j) + (y+k) * surface->w];//   get_pixel_data(x+j, y+k, surface);
			acc += pixel * matrix[((j+1) + (k+1) * size)];
		}
	}

	return acc;
}

void Convolver::run(SDL_Surface* surface, SDL_Surface* new_surface)
{
	for (int y = 0; y < surface->h; y++) {
		for (int x = 0; x < surface->w; x++) {
			set_pixel_data(x, y, new_surface, convolve(x, y, surface));
		}
	}
}
