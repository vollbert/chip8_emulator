#include "chip8.h"

const uint8 FONTSET[80] =
  {
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

void chip8_init(cpu *chip8)
{
  chip8->pc = 0x200;
  chip8->opcode = 0;
  chip8->i      = 0;
  chip8->sp     = 0;
  chip8->screen_flag = 0;

  //load fontset
  memset(chip8->memory, 0, sizeof(chip8->memory));
  memset(chip8->v, 0, sizeof(chip8->v));
  memset(chip8->stack, 0, sizeof(chip8->stack));
  memset(chip8->keyboard, 0, sizeof(chip8->keyboard));
  memset(chip8->screen, 0, sizeof(chip8->screen));

  for (int i = 0; i < 80; i++)
  {
    chip8->memory[i] = FONTSET[i];
  }

  
}

int load_rom(cpu *chip8, char *file)
{
  FILE *fp = fopen(file, "rb");

  if (fp == NULL)
  {
    printf("Could not read file\n");
    return 0;
  }

  fseek(fp , 0, SEEK_END);
  int size = ftell(fp);
  fseek(fp , 0, SEEK_SET);

  fread(chip8->memory + 0x200, size, 1, fp); 

  fclose(fp);
  return 1;
}



void execute_instruction(cpu *chip8)
{
  uint8 v_x = chip8->v[(chip8->opcode & 0x0F00) >> 8];
  fetch_opcode(chip8);
  // first compare to the first hex value
  switch (chip8->opcode & 0xF000) {
    case 0x0000:

      switch (chip8->opcode) {

        case 0x00E0:
          // 00E0 - CLS Clear the display.
          clear_display(chip8);
          chip8->pc += 2;
          chip8->screen_flag = 1;
          break;

        case 0x00EE:
          //00EE - RET Return from a subroutine.
          chip8->sp -= 1;
          chip8->pc = chip8->stack[chip8->sp] + 2;
          break;
      }
      break;

    case 0x1000:
      // jump to address nnn
      chip8->pc = chip8->opcode & 0x0FFF;
      break;
    case 0x2000:
      // 2nnn - CALL addr Call subroutine at nnn.
      chip8->stack[chip8->sp] = chip8->pc;
      chip8->pc = chip8->opcode & 0x0FFF;
      chip8->sp += 1;
      break;
    case 0x3000:
      // 3xkk - SE Vx, byte Skip next instruction if Vx = kk.
      if (chip8->v[(chip8->opcode & 0x0F00) >> 8] == (chip8->opcode & 0x00FF)) 
      {
        chip8->pc += 4;
      }
      else {
        chip8->pc += 2;
      }
      break;
    case 0x4000:
      // 4xkk - SNE Vx, byte Skip next instruction if Vx != kk.
      if (chip8->v[(chip8->opcode & 0x0F00) >> 8] != (chip8->opcode & 0x00FF)) 
      {
        chip8->pc += 4;
      }
      else {
        chip8->pc += 2;
      }
      break;
    case 0x5000:
      // 5xy0 - SE Vx, Vy Skip next instruction if Vx = Vy.
      if (chip8->v[(chip8->opcode & 0x0F00) >> 8] == chip8->v[(chip8->opcode & 0x00F0) >> 4]) 
      {
        chip8->pc += 4;
      }
      else {
        chip8->pc += 2;
      }
      break;
    case 0x6000:
      // 6xkk add value of kk to register Vx and stores value in Vx
      uint8 target_reg = (chip8->opcode & 0x0F00) >> 8;
      chip8->v[target_reg] = chip8->opcode & 0x00FF;
      chip8->pc += 2;
      break;
    case 0x7000:
      // 7xkk - ADD Vx, byte Set Vx = Vx + kk.
      chip8->v[(chip8->opcode & 0x0F00) >> 8] += chip8->opcode & 0x00FF;
      chip8->pc += 2;
      break;
    case 0x8000:

      switch (chip8->opcode & 0x000F) 
      {
        case 0x0000:
          // 8xy0 - LD Vx, Vy Set Vx = Vy.
          chip8->v[(chip8->opcode & 0x0F00) >> 8] = chip8->v[(chip8->opcode & 0x00F0) >> 4];
          chip8->pc += 2;
          break;
        case 0x0001:
          // 8xy1 - OR Vx, Vy Set Vx = Vx OR Vy.
          chip8->v[(chip8->opcode & 0x0F00) >> 8] |= chip8->v[(chip8->opcode & 0x00F0) >> 4];
          chip8->pc += 2;
          break;
        case 0x0002:
          // 8xy2 - AND Vx, Vy Set Vx = Vx AND Vy.
          chip8->v[(chip8->opcode & 0x0F00) >> 8] &= chip8->v[(chip8->opcode & 0x00F0) >> 4];
          chip8->pc += 2;
          break;
        case 0x0003:
          // 8xy3 - XOR Vx, Vy Set Vx = Vx XOR Vy.
          chip8->v[(chip8->opcode & 0x0F00) >> 8] ^= chip8->v[(chip8->opcode & 0x00F0) >> 4];
          chip8->pc += 2;
          break;
        case 0x0004:
          // 8xy4 - ADD Vx, Vy
          // Set Vx = Vx + Vy, set VF = carry.
          chip8->v[(chip8->opcode & 0x0F00) >> 8] += chip8->v[(chip8->opcode & 0x00F0) >> 4];
          if ((chip8->v[(chip8->opcode & 0x0F00) >> 8]) > 255)
          {
            // set carry
            chip8->v[0xF] = 1;
          }
          chip8->pc += 2;
          break;
        case 0x0005:
          // 8xy5 - sub vx, vy
          // set vx = vx - vy, set vf = not borrow.
          if (chip8->v[(chip8->opcode & 0x0F00) >> 8] > chip8->v[(chip8->opcode & 0x00F0) >> 4])
          {
            chip8->v[0xF] = 1;
          }
          else 
        {
            chip8->v[0xF] = 0;
          }
          chip8->v[(chip8->opcode & 0x0F00) >> 8] -= chip8->v[(chip8->opcode & 0x00F0) >> 4];
          chip8->pc += 2;
          break;
        case 0x0006:
          // 8xy6 - SHR Vx {, Vy}
          // Set Vx = Vx SHR 1.
          if ((chip8->v[(chip8->opcode & 0x0F00) >> 8] & 0x01) != 0)
          {
            chip8->v[0xF] = 1;
          }
          else 
        {
            chip8->v[0xF] = 0;
          }
          chip8->v[(chip8->opcode & 0x0F00) >> 8] = chip8->v[(chip8->opcode & 0x0F00) >> 8] >> 1;
          chip8->pc += 2;
          break;
        case 0x0007:
          // 8xy7 - SUBN Vx, Vy
          // Set Vx = Vy - Vx, set VF = NOT borrow.
          if (chip8->v[(chip8->opcode & 0x0F00) >> 8] < chip8->v[(chip8->opcode & 0x00F0) >> 4])
          {
            chip8->v[0xF] = 1;
          }
          else 
        {
            chip8->v[0xF] = 0;
          }
          chip8->v[(chip8->opcode & 0x00F0) >> 4] = chip8->v[(chip8->opcode & 0x00F0) >> 4] >> 1;
          chip8->pc += 2;
          break;
        case 0x000E:
          // 8xyE - SHL Vx {, Vy}
          // Set Vx = Vx SHL 1.
          if ((chip8->v[(chip8->opcode & 0x0F00) >> 8] & 0x80) != 0)
          {
            chip8->v[0xF] = 1;
          }
          else 
        {
            chip8->v[0xF] = 0;
          }
          chip8->v[(chip8->opcode & 0x0F00) >> 8] = chip8->v[(chip8->opcode & 0x0F00) >> 8] << 1;
          chip8->pc += 2;
          break;
      }
      break;
    case 0x9000:
      // 9xy0 - SNE Vx, Vy
      // Skip next instruction if Vx != Vy.
      if (chip8->v[(chip8->opcode & 0x0F00) >> 8] != chip8->v[(chip8->opcode & 0x00F0) >> 4])
      {
        chip8->pc += 4;
      }
      else 
    {
        chip8->pc += 2;
      }
      break;
      break;
    case 0xA000:
      // Annn - LD I, addr
      // Set I = nnn.
      chip8->i = chip8->opcode & 0x0FFF;
      chip8->pc += 2;
      break;
    case 0xB000:
      // Bnnn - JP V0, addr
      // Jump to location nnn + V0.
      chip8->pc = (chip8->opcode & 0x0FFF) + chip8->v[0];
      break;
    case 0xC000:
      // Cxkk - RND Vx, byte
      // Set Vx = random byte AND kk.
      chip8->v[(chip8->opcode & 0x0F00) >> 8] = (rand() % 255) & (chip8->opcode & 0x00FF);
      chip8->pc += 2;
      break;
    case 0xD000:
      // Dxyn - DRW Vx, Vy, nibble
      // Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
      int x = chip8->v[(chip8->opcode & 0x0F00) >> 8] % 64;
      int y = chip8->v[(chip8->opcode & 0x00F0) >> 4] % 32;
      int n = chip8->opcode & 0x000F;
      chip8->v[0xF] = 0;
      uint8 sprite_data;

      for (int i = 0; i < n; i++)
      {

        if (y + i > 32)
        {
          break;
        }

        sprite_data = chip8->memory[chip8->i + i];

        for (int j = 0; j < 8 ; j++)
        {

          if (x + j > 64)
          {
            break;
          }

          if ((sprite_data & (0x80 >> j)) != 0)
          {
            // current pixel is on
            if (chip8->screen[x + j + 64 * (y +i)] == 1) 
            {
              chip8->v[0xF] = 1;
            }
            chip8->screen[x + j + 64 * (y +i)] ^= 1;
          }
        }

      }
      chip8->screen_flag = 1;
      chip8->pc += 2;


      break;
    case 0xE000:
      switch (chip8->opcode & 0x00FF) 
      {
        case 0x9E:
          // Ex9E - SKP Vx
          // Skip next instruction if key with the value of Vx is pressed.
          if (chip8->keyboard[chip8->v[(chip8->opcode & 0x0F00) >> 8]] != 0)
          {
            chip8->pc += 4;
          }
          break;
        case 0xA1:
          // ExA1 - SKNP Vx
          // Skip next instruction if key with the value of Vx is not pressed.
          if (chip8->keyboard[chip8->v[(chip8->opcode & 0x0F00) >> 8]] == 0)
          {
            chip8->pc += 4;
          }
          break;
      } 
      break;
    case 0xF000:
      switch (chip8->opcode & 0x00FF) {
        case 0x07:
          // Fx07 - LD Vx, DT
          // Set Vx = delay timer value.
          chip8->v[(chip8->opcode & 0x0F00) >> 8] = chip8->delay_timer;
          chip8->pc += 2;
          break;
        case 0x0A:
          // Fx0A - LD Vx, K
          // Wait for a key press, store the value of the key in Vx.
          // to do
          break;
        case 0x15:
          // Fx15 - LD DT, Vx
          // Set delay timer = Vx.
          chip8->delay_timer = chip8->v[(chip8->opcode & 0x0F00) >> 8];
          chip8->pc += 2;
          break;
        case 0x18:
          // Fx18 - LD ST, Vx
          // Set sound timer = Vx.
          chip8->sound_timer = chip8->v[(chip8->opcode & 0x0F00) >> 8];
          chip8->pc += 2;
          break;
        case 0x1E:
          // Fx1E - ADD I, Vx
          // Set I = I + Vx.
          chip8->i += chip8->v[(chip8->opcode & 0x0F00) >> 8];
          chip8->pc += 2;
          break;
        case 0x29:
          // Fx29 - LD F, Vx
          // Set I = location of sprite for digit Vx.
          v_x = chip8->v[(chip8->opcode & 0x0F00) >> 8];
          chip8->i = 0x50 + v_x - 48 * 5;
          chip8->pc += 2;
          break;
        case 0x33:
          // Fx33 - LD B, Vx
          // Store BCD representation of Vx in memory locations I, I+1, and I+2.
          v_x = chip8->v[(chip8->opcode & 0x0F00) >> 8];
          uint8 v_x1 = v_x % 10;
          uint8 v_x10 = (v_x / 10) % 10;
          uint8 v_x100 = v_x / 100;
          uint8 i = chip8->i;
          chip8->memory[i] = v_x100;
          chip8->memory[i + 1] = v_x10;
          chip8->memory[i + 2] = v_x1;
          chip8->pc += 2;
          break;
        case 0x55:
          // Fx55 - LD [I], Vx
          // Store registers V0 through Vx in memory starting at location I.
          uint8 x = (chip8->opcode & 0x0F00) >> 8;
          for (int i = 0; i < x; i++) {
            chip8->memory[chip8->i + i] = chip8->v[i];
          }
          chip8->pc += 2;
          break;
      }
      break;
  }
}

void get_screen_buffer(cpu *chip8, uint32 *buffer)
{
  for (int y_value = 0; y_value < 32; y_value ++) {
    for (int x_value = 0; x_value < 64; x_value ++) {
      buffer[x_value + 64 * y_value] = (uint32)chip8->screen[x_value + 64 * y_value] * 0xFFFFFFFF;
    }
  }
}

void update_screen(uint32 *buffer,SDL_Texture **texture, SDL_Renderer **renderer)
{
  SDL_UpdateTexture(*texture, NULL, buffer, 64 * sizeof(uint32));
  SDL_RenderClear(*renderer);
  SDL_RenderCopy(*renderer, *texture, NULL, NULL);
  SDL_RenderPresent(*renderer);
}

void fetch_opcode(cpu *chip8)
{
  chip8->opcode = chip8->memory[chip8->pc] << 8 | chip8->memory[chip8->pc + 1];
}

 void clear_display(cpu *chip8){
  for (int y_value = 0; y_value < 32; y_value ++) {
    for (int x_value = 0; x_value < 64; x_value ++) {
      chip8->screen[x_value + 64 * y_value] = 0;
    }
  }
}

