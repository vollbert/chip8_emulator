#ifndef CHIP8_H
#define CHIP8_H

#include <stdio.h>
#include <SDL2/SDL.h>

typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
typedef struct cpu_t cpu;


struct cpu_t {
  uint16 opcode;
  // 0x000 - 0x1FF contains font set in emu
  // 0x050 - 0x0A0 Used for the build in 4x5 pixel font set (0-F)
  // 0x200 - 0xFFF Program ROM and work RAM
  uint8 memory[4096];
  uint8 v[16];
  uint16 i;
  uint8 delay_timer;
  uint8 sound_timer;
  uint16 pc;                // programm counter
  uint8 sp;                 // stack pointer
  uint16 stack[16];
  uint8 keyboard[16];
  uint8 screen[64 * 32];
  uint8 screen_flag;
};

void chip8_init(cpu *chip8);
void get_screen_buffer(cpu *chip8, uint32 *buffer);

int load_rom(cpu *chip8,char *file);
void fetch_opcode(cpu *chip8);
void execute_instruction(cpu *chip8);

void emulate_cycle();

void draw_graphics();

void set_keys();

void update_screen();
void clear_display(cpu *chip8);

#endif // !CHIP8_H
