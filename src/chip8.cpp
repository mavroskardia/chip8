#include <iostream>
#include <iomanip>
#include <iterator>
#include <vector>
#include <fstream>
#include <chrono>
#include <thread>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <unordered_map>


#include "Chip8.h"
#include "SDL.h"
#include "SDL_ttf.h"

namespace Chip8
{
	using namespace std;

	const int NumPixelsWide = 64;
	const int NumPixelsTall = 32;
	const int ScreenWidth = 1280;
	const int ScreenHeight = 480;
	const int GameWidth = 640;
	const int GameHeight = 480;
	const int PixelWidth = GameWidth / NumPixelsWide;
	const int PixelHeight = GameHeight / NumPixelsTall;

	// 123C 456D 789E A0BF   -- but we'll use:
    // 1234 qwer asdf zxcv
	const unordered_map<char, int> keymap = {
		{'1', 1},   {'2', 2}, {'3', 3},   {'4', 0xc},
		{'q', 4},   {'w', 5}, {'e', 6},   {'r', 0xd},
		{'a', 7},   {'s', 8}, {'d', 9},   {'f', 0xe},
		{'z', 0xa}, {'x', 0}, {'c', 0xb}, {'v', 0xf}
	};

	Chip8::Chip8()
	{
	}

	Chip8::~Chip8()
	{
		SDL_DestroyWindow((SDL_Window*)window);
		SDL_DestroyRenderer((SDL_Renderer*)renderer);
		SDL_DestroyTexture((SDL_Texture*)texture);
		SDL_FreeFormat((SDL_PixelFormat*)pixel_format);
		TTF_CloseFont((TTF_Font*)font);
		
		if (pixels)
			delete[] pixels;
			
		TTF_Quit();
		SDL_Quit();
	}

	bool Chip8::load(string filename)
	{
		ifstream file(filename, ios::in | ios::binary | ios::ate); // ate == at the end

		if (file.is_open()) {
			program_length = file.tellg();
			file.seekg(0, ios::beg);
			program = vector<byte>(program_length, 0);

			file.read((char*) &program[0], program_length);
			file.close();

			if (program_length == program.size())
				cout << "successfully read in " << program.size() << " bytes" << endl;

			return true;
		}

		return false;
	}

	void Chip8::decompile() {
		initialize_memory();

		while (pc <= program_length-1) {
			word oc = get_next_instruction();
			cout << "0x" << hex << pc << ": " << translate_opcode(oc) << endl;
			pc += 2;
		}
	}

	string Chip8::translate_opcode(word oc) {
		word vx = (oc & 0x0f00) >> 8;
		word vy = (oc & 0x00f0) >> 4;
		word addr = oc & 0x0fff;
		word nn = oc & 0x00ff;
		word n = oc & 0x000f;

		stringstream ss;

		switch (oc & 0xf000) {
			case 0x1000: ss << "JP   0x" << hex << addr; break;
			case 0x2000: ss << "CALL " << addr; break;
			case 0x3000: ss << "SE   V"  << hex << vx << " 0x" << hex << nn; break;
			case 0x4000: ss << "SNE  V" << hex << vx << " 0x" << hex << nn; break;
			case 0x5000: ss << "SE   V" << hex << vx << " V" << vy; break;
			case 0x6000: ss << "LD   V" << hex << vx << " 0x" << hex << nn; break;
			case 0x7000: ss << "ADD  V" << hex << vx << " 0x" << hex << nn; break;
			case 0x9000: ss << "SNE  V" << hex << vx << " V" << vy; break;
			case 0xa000: ss << "LD   I  0x" << hex << addr; break;
			case 0xb000: ss << "JP   V0 0x" << hex << addr; break;
			case 0xc000: ss << "RND  V" << hex << vx << ' ' << hex << nn; break;
			case 0xd000: ss << "DRW  V" << hex << vx << " V" << vy << " " << n; break;
			case 0x0000: {
				switch (oc & 0x00ff) {
					case 0xe0: ss << "CLS"; break;
					case 0xee: ss << "RET"; break;
					case 0xf0: ss << "SET SCHIP (NOT SUPPORTED)"; break;
					default: ss << "NOP  0x" << hex << oc;
				}
				break;
			}
			case 0x8000: {
				switch (oc & 0x000f) {
					case 0x0: ss << "LD   V" << hex << vx << " V" << vy; break;
					case 0x1: ss << "OR   V" << hex << vx << " V" << vy; break;
					case 0x2: ss << "AND  V" << hex << vx << " V" << vy; break;
					case 0x3: ss << "XOR  V" << hex << vx << " V" << vy; break;
					case 0x4: ss << "ADD  V" << hex << vx << " V" << vy; break;
					case 0x5: ss << "SUB  V" << hex << vx << " V" << vy; break;
					case 0x6: ss << "SHR  V" << hex << vx; break;
					case 0x7: ss << "SUBN V" << hex << vx << " V" << vy; break;
					case 0xe: ss << "SHL  V" << hex << vx; break;
					default: ss << "NOP  0x" << hex << oc;
				}
				break;
			}
			case 0xf000: {
				switch (oc & 0x00ff) {
					case 0x07: ss << "LD   V" << hex << vx << " DT"; break;
					case 0x0a: ss << "LD   V" << hex << vx << " K"; break;
					case 0x15: ss << "LD   DT V" << hex << vx; break;
					case 0x18: ss << "LD   ST V" << hex << vx; break;
					case 0x1e: ss << "ADD  I V" << hex << vx; break;
					case 0x29: ss << "LD   F V" << hex << vx; break;
					case 0x33: ss << "LD   B V" << hex << vx; break;
					case 0x55: ss << "LD   [I] V" << hex << vx; break;
					case 0x65: ss << "LD   V" << hex << vx << " [I]"; break;
					default: ss << "NOP  0x" << hex << oc;
				}
				break;
			}
			case 0xe000: {
				switch (oc & 0x00ff) {
					case 0x9e: ss << "SKP  V" << hex << vx; break;
					case 0xa1: ss << "SKNP V" << hex << vx; break;
					default: ss << "NOP  0x" << hex << oc;
				}
				break;
			}
			default: ss << "NOP  0x" << hex << oc;
		}

		return ss.str();
	}

	void Chip8::run() {
		show_debug = false;

		initialize_memory();

		if (initialize_screen() != 0) return;

		running = true;

		thread timer_thread([this] () {
			while (running) {
				update_timers();
				// 60 Hz (ish)
				SDL_Delay(1000/60);
			}
		});

		while (running) {
			SDL_Delay(show_debug ? 1 : 2);

			render();
			update_events();

			if (waiting_for_keypress || paused) continue;

			auto opcode = get_next_instruction();
			auto opcode_string = translate_opcode(opcode);

			opcode_count[opcode_string]++;
			previous_opcodes.push_back(opcode_string);

			if (show_debug)
				cout << translate_opcode(opcode) << endl; //<< "\t" << this->register_values_stream() << endl;

			execute_instruction(opcode);
		}

		timer_thread.join();
	}

	void Chip8::update_events() {
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_KEYDOWN:
					if (keymap.count(event.key.keysym.sym) > 0) {
						waiting_for_keypress = false;
						keys_pressed[keymap.at((char)event.key.keysym.sym)] = true;
						if (keypress_register != -1) {
							v[keypress_register] = keymap.at(event.key.keysym.sym);
							keypress_register = -1;
						}
					} else {
						switch (event.key.keysym.sym) {
							case SDLK_ESCAPE:
								running = false;
								break;
							case SDLK_p:
								paused = !paused;
								break;
							case SDLK_o:
								show_debug = !show_debug;
								break;
							case SDLK_i:
								stringstream ss;
								for (auto& x : opcode_count) {
									ss << x.first << ": " << x.second << endl;
								}
								cout << ss.str() << endl;
								break;
						}
					}
					break;
				case SDL_KEYUP:
					if (keymap.count(event.key.keysym.sym) > 0) {
						keys_pressed[keymap.at((char)event.key.keysym.sym)] = false;
					}
					break;
				case SDL_QUIT:
					running = false;
					break;
			}
		}
	}

	void Chip8::update_timers() {
		if (dt > 0) dt--;
		if (st > 0) {
			if (st-- == 1) {
				cout << "BEEP!" << endl;
			}
		}
	}

	void Chip8::render() {
		auto ren = (SDL_Renderer*) renderer;

		if (!redraw && !show_debug) return;

		clear_window();

		SDL_SetRenderDrawColor(ren, 100, 100, 0, 255);

		for (int y = 0; y < NumPixelsTall; y++) {
			for (int x = 0; x < NumPixelsWide; x++) {
				if (gfx[x + (NumPixelsWide * y)]) {					
					fill_pixels(x, y);
				}
			}
		}

		if (show_debug) render_debug();

		postprocess(ren);

		SDL_RenderPresent(ren);

		redraw = false;
	}
	
	inline void Chip8::fill_pixels(int gx, int gy) {
		SDL_Rect r = { gx * PixelWidth, gy * PixelHeight, PixelWidth, PixelHeight };					
				
		for (int x = r.x; x < r.x + r.w; x++) {
			for (int y = r.y; y < r.y + r.h; y++) {
				pixels[x + y * GameWidth] = SDL_MapRGBA((SDL_PixelFormat*)pixel_format, 100, 0, 0, 0xff);
			}
		}
	}	
		
	void Chip8::postprocess(void* renderer) {
		auto ren = (SDL_Renderer*) renderer;
		auto tex = (SDL_Texture*) texture;
		auto pf = (SDL_PixelFormat*) pixel_format;

		uint32_t p0,p1,p2,p3,p4,p5,p6,p7,p8;
		uint8_t r,g,b,a;
		
		// skip tricky boundaries... because.
		for (int y = 1; y < GameHeight-1; y++) {
			for (int x = 1; x < GameWidth-1; x++) {
				// box blur is simplest
				p0 = pixels[(x-1) + (y-1) * GameWidth];
				p1 = pixels[x + (y-1) * GameWidth];
				p2 = pixels[(x+1) + (y-1) * GameWidth];
				p3 = pixels[(x-1) + y * GameWidth];
				p4 = pixels[x + y * GameWidth];
				p5 = pixels[(x+1) + y * GameWidth];
				p6 = pixels[(x-1) + (y+1) * GameWidth];
				p7 = pixels[x + (y+1) * GameWidth];
				p8 = pixels[(x+1) + (y+1) * GameWidth];
				
				SDL_GetRGBA((p0 + p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8) / 9, pf, &r, &g, &b, &a);
				
				pixels[x + y * GameWidth] = SDL_MapRGBA(pf, r, g, b, a);
			}
		}
		
		
		SDL_Rect dstrect = {0,0,GameWidth,GameHeight};
		SDL_UpdateTexture(tex, nullptr, pixels, GameWidth*4*sizeof(byte));
		SDL_RenderCopy(ren, tex, nullptr, &dstrect);
	}

	void Chip8::render_debug() {
		auto ren = (SDL_Renderer*) renderer;
		if (!show_debug) return;

		vector<string> dbgout;

		for (int i=0;i<8;i++) {
			stringstream ss;
			ss << hex << i << ": " << setw(3) << hex << (int) v[i] << " " << hex << (i+8) << ": " << setw(3) << hex << (int) v[i+8];
			dbgout.push_back(ss.str());
		}

		stringstream ss, ss2;

		ss << "PC: " << setw(4) << hex << (int) pc << " ";
		ss << "DT: " << setw(4) << hex << (int) dt << " ";

		dbgout.push_back(ss.str());

		ss2 << " I: " << setw(4) << hex << (int) i  << " ";
		ss2 << "ST: " << setw(4) << hex << (int) st << " ";

		dbgout.push_back(ss2.str());

		dbgout.push_back("Disassembly:");

		for (string oc : previous_opcodes) {
			dbgout.push_back(oc);
		}

		if (previous_opcodes.size() > 8) {
			previous_opcodes.pop_front();
		}

		int y = 0;

		for (auto s : dbgout) {
			auto debug_surface = TTF_RenderUTF8_Blended((TTF_Font*)font, s.c_str(), {200,200,200});
			auto debug_texture = SDL_CreateTextureFromSurface(ren, debug_surface);
			SDL_Rect r = {GameWidth + 50, y, debug_surface->w, debug_surface->h};
			y += debug_surface->h + 5;
			SDL_RenderCopy(ren, debug_texture, NULL, &r);
			SDL_FreeSurface(debug_surface);
		}
	}

	string Chip8::register_values_stream() {
		stringstream ss;

		for (int i = 0; i < 0xf; i++) {
			stringstream ts;
			ts << "V" << hex << i;
			ss << setw(4) << ts.str();
		}

		ss << setw(4) << "PC" << setw(4) << "I" << setw(4) << "DT" << setw(4) << "ST" << endl;

		for (int i = 0; i < 0xf; i++) {
			stringstream tmpss;
			tmpss << hex << (int) v[i];
			ss << setw(4) << tmpss.str();
		}

		ss << setw(4) << hex << pc << setw(4) << hex << i << setw(4) << hex << (int) dt << setw(4) << hex << (int) st;

		return ss.str();
	}

	int Chip8::initialize_screen() {
		if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {\
			cerr << "SDL_Init Error: " << SDL_GetError() << endl;
			return 1;
		}

		if (TTF_Init() != 0) {
			cerr << "TTF_Init Error: " << TTF_GetError() << endl;
			return 1;
		}

		window = SDL_CreateWindow("Chip-8 Emulator", 100, 100, ScreenWidth, ScreenHeight, SDL_WINDOW_SHOWN);

		if (window == nullptr) {
			cerr << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
			return 1;
		}

		renderer = SDL_CreateRenderer((SDL_Window*)window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

		if (renderer == nullptr) {
			SDL_DestroyWindow((SDL_Window*)window);
			cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << endl;
			return 1;
		}
		
		texture = SDL_CreateTexture((SDL_Renderer*)renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, GameWidth, GameHeight);
		
		pixel_format = SDL_AllocFormat(SDL_GetWindowPixelFormat((SDL_Window*)window));
		
		pixels = new uint32_t[GameWidth*GameHeight];
		
		memset(pixels, 0xff, GameWidth*GameHeight);
		
		font = (TTF_Font*) TTF_OpenFont("DroidSansMono.ttf", 16);

		clear_window();

		SDL_RenderPresent((SDL_Renderer*)renderer);

		return 0;
	}

	void Chip8::clear_window() {
		memset(pixels, 0xff, GameWidth*GameHeight);
		SDL_SetRenderDrawColor((SDL_Renderer*)renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear((SDL_Renderer*)renderer);
		redraw = true;
	}

	void Chip8::initialize_memory() {
		mem.clear();
		mem.resize(0xfff);

		v.clear();
		v.resize(0x16, 0);

		gfx.clear();
		gfx.resize(64 * 32);

		stack.clear();

		keys_pressed.clear();
		keys_pressed.resize(0xf, false);

		st = 0;
		dt = 0;

		byte font[] = {
			0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
			0x20, 0x60, 0x20, 0x20, 0x70, // 1
			0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
			0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
			0x90, 0x90, 0xF0, 0x10, 0x10, // 4
			0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
			0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
			0xF0, 0x10, 0x20, 0x40, 0x40, // 7
			0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
			0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
			0xF0, 0x90, 0xF0, 0x90, 0x90, // A
			0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
			0xF0, 0x80, 0x80, 0x80, 0xF0, // C
			0xE0, 0x90, 0x90, 0x90, 0xE0, // D
			0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
			0xF0, 0x80, 0xF0, 0x80, 0x80  // F
		};

		copy(begin(font), end(font), begin(mem));

		auto it = begin(mem);
		advance(it, 0x200);
		copy(begin(program), end(program), inserter(mem, it));
	}

	inline word Chip8::get_next_instruction() {
		return (mem[pc] << 8) | mem[pc+1];
	}

	void Chip8::execute_instruction(word oc) {
 		pc += 2;

		auto vx   = (oc & 0x0f00) >> 8;
		auto vy   = (oc & 0x00f0) >> 4;
		auto addr =  oc & 0x0fff;
		auto nn   =  oc & 0x00ff;
		auto n    =  oc & 0x000f;

		switch (oc & 0xf000) {
			case 0x1000: return jump_to(addr);
			case 0x2000: return jump_to_subroutine(addr);
			case 0x3000: return skip_if_equal(vx, nn);
			case 0x4000: return skip_if_not_equal(vx, nn);
			case 0x5000: return skip_if_registers_equal(vx, vy);
			case 0x6000: return set_register(vx, nn);
			case 0x7000: return add_to_register(vx, nn);
			case 0x9000: return skip_if_registers_not_equal(vx, vy);
			case 0xa000: return set_index(addr);
			case 0xb000: return jump_to_addr_plus_register0(addr);
			case 0xc000: return set_register_to_random(vx, nn);
			case 0xd000: return draw_to_screen(vx, vy, n);
			case 0x0000:
				switch (oc & 0x00ff) {
					case 0xe0: return clear_screen();
					case 0xee: return return_from_subroutine();
					case 0xf0: return set_schip();
					default: return nop(oc);
				}
				break;
			case 0x8000:
				switch (oc & 0x000f) {
					case 0x0: return register_copy(vx, vy);
					case 0x1: return or_registers(vx, vy);
					case 0x2: return and_registers(vx, vy);
					case 0x3: return xor_registers(vx, vy);
					case 0x4: return add_registers(vx, vy);
					case 0x5: return subtract_registers(vx, vy);
					case 0x6: return shift_register_right(vx);
					case 0x7: return reverse_subtract_register(vx, vy);
					case 0xe: return shift_register_left(vx);
					default: return nop(oc);
				}
				break;
			case 0xf000:
				switch (oc & 0x00ff) {
					case 0x07: return set_register_to_dt(vx);
					case 0x0a: return wait_for_keypress(vx);
					case 0x15: return set_delay_timer(vx);
					case 0x18: return set_sound_timer(vx);
					case 0x1e: return add_register_to_index(vx);
					case 0x29: return point_i_to_font(vx);
					case 0x33: return store_bcd(vx);
					case 0x55: return save_registers(vx);
					case 0x65: return fill_registers(vx);
					default: return nop(oc);
				}
				break;
			case 0xe000:
				switch (oc & 0x00ff) {
					case 0x9e: return skip_if_key_pressed(vx);
					case 0xa1: return skip_if_key_not_pressed(vx);
					default: return nop(oc);
				}
				break;
			default:
				return nop(oc);
		}
	}

	inline void Chip8::jump_to(word addr) {
		pc = addr;
	}

	inline void Chip8::jump_to_subroutine(word addr) {
		stack.push_back(pc);
		pc = addr;
	}

	inline void Chip8::skip_if_equal(word vx, word nn) {
		if (v[vx] == nn) pc += 2;
	}

	inline void Chip8::skip_if_not_equal(word vx, word nn) {
		if (v[vx] != nn) pc += 2;
	}

	inline void Chip8::skip_if_registers_equal(word vx, word vy) {
		if (v[vx] == v[vy]) pc += 2;
	}

	inline void Chip8::set_register(word vx, word nn) {
		v[vx] = nn;
	}

	inline void Chip8::add_to_register(word vx, word nn) {
		v[vx] += nn;
	}

	inline void Chip8::skip_if_registers_not_equal(word vx, word vy) {
		if (v[vx] != v[vy]) pc += 2;
	}

	inline void Chip8::set_index(word addr) {
		i = addr;
	}

	inline void Chip8::jump_to_addr_plus_register0(word addr) {
		pc = v[0] + addr;
	}

	inline void Chip8::set_register_to_random(word vx, word nn) {
		auto rnd = (rand() * 0xff) & 0xff;
		v[vx] = rnd & nn;
	}

	void Chip8::draw_to_screen(word vx, word vy, word n) {
		word x = v[vx];
		word y = v[vy];
		word spr, idx;

		v[0xf] = 0;

		for (word i = 0; i < n; i++) {
			spr = mem[this->i + i];

			for (word j = 0; j < 8; j++) {
				if ((spr & (0x80 >> j))) {
					idx = x + j + ((y + i) * 64);
					if (gfx[idx]) v[0xf] = 1;
					gfx[idx] = gfx[idx] ^ 1;
				}
			}
		}

		redraw = true;
	}

	inline void Chip8::clear_screen() {
		gfx.clear();
		gfx.resize(64 * 32);
		clear_window();
	}

	inline void Chip8::return_from_subroutine() {
		pc = stack.back();
		stack.pop_back();
	}

	inline void Chip8::set_schip() {
		cout << "SCHIP not supported" << endl;
	}

	inline void Chip8::register_copy(word vx, word vy) {
		v[vx] = v[vy];
	}

	inline void Chip8::or_registers(word vx, word vy) {
		v[vx] |= v[vy];
	}

	inline void Chip8::and_registers(word vx, word vy) {
		v[vx] &= v[vy];
	}

	inline void Chip8::subtract_registers(word vx, word vy) {
		v[vx] -= v[vy];
		v[0xf] = v[vx] < v[vy] ? 1 : 0;
	}

	inline void Chip8::shift_register_right(word vx) {
		v[0xf] = v[vx] & 1;
		v[vx] = v[vx] >> 1;
	}

	inline void Chip8::reverse_subtract_register(word vx, word vy) {
		v[vx] = v[vy] - v[vx];
		v[0xf] = v[vx] > v[vy] ? 1 : 0;
	}

	inline void Chip8::xor_registers(word vx, word vy) {
		v[vx] ^= v[vy];
	}

	inline void Chip8::add_registers(word vx, word vy) {
		v[vx] += v[vy];
		v[0xf] = (int) v[vx] > 0xff;
		v[vx] &= 255;
	}

	inline void Chip8::shift_register_left(word vx) {
		v[0xf] = v[vx] & 0x80 ? 1 : 0;
		v[vx] <<= 1;
	}

	void Chip8::nop(word opcode) {
		stringstream ss;

		ss << "Invalid opcode: 0x" << hex << opcode;

		cout << ss.str() << endl;
	}

	inline void Chip8::set_register_to_dt(word vx) {
		v[vx] = dt;
	}

	inline void Chip8::wait_for_keypress(word vx) {
		keypress_register = vx;
		waiting_for_keypress = true;
	}

	inline void Chip8::set_delay_timer(word vx) {
		dt = v[vx];
	}

	inline void Chip8::set_sound_timer(word vx) {
		st = v[vx];
	}

	inline void Chip8::add_register_to_index(word vx) {
		i += v[vx];
	}

	inline void Chip8::point_i_to_font(word vx) {
		i = v[vx] * 5;
	}

	void Chip8::store_bcd(word vx) {
		byte vxval = v[vx];
		byte hundreds = vxval / 100;
		byte tens = (vxval - hundreds * 100) / 10;
		byte ones = vxval - hundreds * 100 - tens * 10;
		mem[i] = hundreds;
		mem[i+1] = tens;
		mem[i+2] = ones;
	}

	inline void Chip8::save_registers(word vx) {
		for (word n = 0; n < vx + 1; n++) {
			mem[i+n] = v[n];
		}
	}

	inline void Chip8::fill_registers(word vx) {
		for (word n = 0; n < vx + 1; n++) {
			v[n] = mem[i + n];
		}
	}

	inline void Chip8::skip_if_key_pressed(word vx) {
		if (keys_pressed[v[vx]]) pc += 2;
	}

	inline void Chip8::skip_if_key_not_pressed(word vx) {
		if (!keys_pressed[v[vx]]) pc += 2;
	}
}
