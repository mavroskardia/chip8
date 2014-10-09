#ifndef __CHIP8RENDERER_H_
#define __CHIP8RENDERER_H_

#include "SDL_ttf.h"

namespace Chip8 {

	const int ScreenWidth = 1280;
	const int ScreenHeight = 480;
	const int GameWidth = 640;
	const int GameHeight = 480;
	const int PixelWidth = GameWidth / NumPixelsWide;
	const int PixelHeight = GameHeight / NumPixelsTall;

	class Chip8Renderer	{
	private:
		void* font;
		void* window;
		void* renderer; // i can't call these what they really are because ld will crash :(

	public:
		int initialize_screen();
		void teardown_screen();
		void clear_window();
		void render();
	}

}

#endif // __CHIP8RENDERER_H_