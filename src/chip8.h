#ifndef CHIP8_H
#define CHIP8_H

namespace Chip8 {
	
	typedef uint8_t byte;
	typedef uint16_t word;
	
	using namespace std;
	
	class Chip8 {
	private:
		word i = 0;
		word sp = 0;
		byte dt = 0;
		byte st = 0;		
		word pc = 0x200;
		
		vector<byte> v;					// registers v0 through v15
		vector<byte> mem;
		vector<bool> gfx;
		vector<word> stack;
		
		byte keypress_register = 0;
		bool waiting_for_keypress = false;
		vector<bool> keys_pressed;		// CHIP8 only has 16 possible key presses		
		void update_events();
		
		vector<byte> program;
		word program_length; 			// memory limits of chip8 don't allow anything higher than this
		
		Convolver* convolver;
		void initialize_memory();
		int initialize_screen();
		void teardown_screen();
		void clear_window();
		void render();
		void postprocess(vector<SDL_Rect>* pixels);
		void fill_pixels(int x, int y, uint8_t val);
		
		bool running;
		bool redraw;
		bool show_debug = false;
		bool paused = false;
		deque<string> previous_opcodes;
		map<string, int> opcode_count;
		
		void render_debug();
		TTF_Font* font;		
		SDL_Window* window;	
		SDL_Renderer* renderer;
		SDL_Texture* texture;
		SDL_Surface* surface;
		SDL_Surface* tmpsurface;
		SDL_PixelFormat* pixel_format;
		uint32_t* pixels;
		
		word get_next_instruction();
		void execute_instruction(word oc);
		string translate_opcode(word oc);
		
		void jump_to(word addr);
		void jump_to_subroutine(word addr);
		void skip_if_equal(word vx, word nn);
		void skip_if_not_equal(word vx, word nn);
		void skip_if_registers_equal(word vx, word vy);
		void set_register(word vx, word nn);
		void add_to_register(word vx, word nn);
		void skip_if_registers_not_equal(word vx, word vy);
		void set_index(word addr);
		void jump_to_addr_plus_register0(word addr);
		void set_register_to_random(word vx, word nn);
		void draw_to_screen(word vx, word vy, word n);
		void clear_screen();
		void return_from_subroutine();
		void set_schip();
		void register_copy(word vx, word vy);
		void or_registers(word vx, word vy);
		void and_registers(word vx, word vy);
		void xor_registers(word vx, word vy);
		void add_registers(word vx, word vy);
		void subtract_registers(word vx, word vy);
		void shift_register_right(word vx);
		void reverse_subtract_register(word vx, word vy);
		void shift_register_left(word vx);
		void set_register_to_dt(word vx);
		void wait_for_keypress(word vx);
		void set_delay_timer(word vx);
		void set_sound_timer(word vx);
		void add_register_to_index(word vx);
		void point_i_to_font(word vx);
		void store_bcd(word vx);
		void save_registers(word vx);
		void fill_registers(word vx);
		void skip_if_key_pressed(word vx);
		void skip_if_key_not_pressed(word vx);
		void nop(word opcode);
		
		string register_values_stream();
		
	public:
		Chip8();
		~Chip8();
		
		bool load(string filename);
		void decompile();
		void run();
		void update_timers();		
	};
}

#endif // CHIP8_H
