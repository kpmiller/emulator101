#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "chip8emu.h"
#include "font4x5.h"

static int DisassembleChip8Op(uint8_t *codebuffer, int pc)
{
	uint8_t *code = &codebuffer[pc];
	//0x200 because all chip-8 programs start at 0x200 in RAM
	printf("%04x %02x %02x ", pc, code[0], code[1]);
	uint8_t firstnib = (code[0] >> 4);
	
	switch (firstnib)
	{
		case 0x0: 
			switch (code[1])
			{
				case 0xe0: printf("%-10s", "CLS"); break;
				case 0xee: printf("%-10s", "RTS"); break;
				default: printf("UNKNOWN 0"); break;
			}
			break;
		case 0x1: printf("%-10s $%01x%02x", "JUMP", code[0]&0xf, code[1]); break;
		case 0x2: printf("%-10s $%01x%02x", "CALL", code[0]&0xf, code[1]); break;
		case 0x3: printf("%-10s V%01X,#$%02x", "SKIP.EQ", code[0]&0xf, code[1]); break;
		case 0x4: printf("%-10s V%01X,#$%02x", "SKIP.NE", code[0]&0xf, code[1]); break;
		case 0x5: printf("%-10s V%01X,V%01X", "SKIP.EQ", code[0]&0xf, code[1]>>4); break;
		case 0x6: printf("%-10s V%01X,#$%02x", "MVI", code[0]&0xf, code[1]); break;
		case 0x7: printf("%-10s V%01X,#$%02x", "ADI", code[0]&0xf, code[1]); break;
		case 0x8:
			{
				uint8_t lastnib = code[1]&0xf;
				switch(lastnib)
				{
					case 0: printf("%-10s V%01X,V%01X", "MOV.", code[0]&0xf, code[1]>>4); break;
					case 1: printf("%-10s V%01X,V%01X", "OR.", code[0]&0xf, code[1]>>4); break;
					case 2: printf("%-10s V%01X,V%01X", "AND.", code[0]&0xf, code[1]>>4); break;
					case 3: printf("%-10s V%01X,V%01X", "XOR.", code[0]&0xf, code[1]>>4); break;
					case 4: printf("%-10s V%01X,V%01X", "ADD.", code[0]&0xf, code[1]>>4); break;
					case 5: printf("%-10s V%01X,V%01X,V%01X", "SUB.", code[0]&0xf, code[0]&0xf, code[1]>>4); break;
					case 6: printf("%-10s V%01X,V%01X", "SHR.", code[0]&0xf, code[1]>>4); break;
					case 7: printf("%-10s V%01X,V%01X,V%01X", "SUB.", code[0]&0xf, code[1]>>4, code[1]>>4); break;
					case 0xe: printf("%-10s V%01X,V%01X", "SHL.", code[0]&0xf, code[1]>>4); break;
					default: printf("UNKNOWN 8"); break;
				}
			}
			break;
		case 0x9: printf("%-10s V%01X,V%01X", "SKIP.NE", code[0]&0xf, code[1]>>4); break;
		case 0xa: printf("%-10s I,#$%01x%02x", "MVI", code[0]&0xf, code[1]); break;
		case 0xb: printf("%-10s $%01x%02x(V0)", "JUMP", code[0]&0xf, code[1]); break;
		case 0xc: printf("%-10s V%01X,#$%02X", "RNDMSK", code[0]&0xf, code[1]); break;
		case 0xd: printf("%-10s V%01X,V%01X,#$%01x", "SPRITE", code[0]&0xf, code[1]>>4, code[1]&0xf); break;
		case 0xe: 
			switch(code[1])
			{
				case 0x9E: printf("%-10s V%01X", "SKIPKEY.Y", code[0]&0xf); break;
				case 0xA1: printf("%-10s V%01X", "SKIPKEY.N", code[0]&0xf); break;
				default: printf("UNKNOWN E"); break;
			}
			break;
		case 0xf: 
			switch(code[1])
			{
				case 0x07: printf("%-10s V%01X,DELAY", "MOV", code[0]&0xf); break;
				case 0x0a: printf("%-10s V%01X", "KEY", code[0]&0xf); break;
				case 0x15: printf("%-10s DELAY,V%01X", "MOV", code[0]&0xf); break;
				case 0x18: printf("%-10s SOUND,V%01X", "MOV", code[0]&0xf); break;
				case 0x1e: printf("%-10s I,V%01X", "ADI", code[0]&0xf); break;
				case 0x29: printf("%-10s I,V%01X", "SPRITECHAR", code[0]&0xf); break;
				case 0x33: printf("%-10s (I),V%01X", "MOVBCD", code[0]&0xf); break;
				case 0x55: printf("%-10s (I),V0-V%01X", "MOVM", code[0]&0xf); break;
				case 0x65: printf("%-10s V0-V%01X,(I)", "MOVM", code[0]&0xf); break;
				default: printf("UNKNOWN F"); break;
			}
			break;
	}
	
	return 2;
}

static void UnimplementedInstruction(Chip8State* state)
{
	//pc will have advanced one, so undo that
	printf ("Error: Unimplemented instruction\n");
	DisassembleChip8Op(state->memory, state->PC);
	printf("\n");
	exit(1);
}


static void Op0(Chip8State *state, uint8_t *code)
{
	switch (code[1])
	{
		case 0xE0: 			//CLS
			memset(state->screen, 0, 64*32/8);
			state->PC+=2;
			break;
		case 0xEE: 
			{
				uint16_t target = (state->memory[state->SP] << 8) | state->memory[state->SP+1];
				state->SP += 2;
				state->PC = target;
			}
			break;
		default:
			UnimplementedInstruction(state); 
			break;
	}
}

static void Op1(Chip8State *state, uint8_t *code)
{
	uint16_t target = ((code[0]&0xf)<<8) | code[1];
	if (target == state->PC)
	{
		printf("Infinite loop detected.  Setting halt flag.\n");
		state->halt = 1;
	}
	state->PC = target;
}

static void Op2(Chip8State *state, uint8_t *code)
{
	state->SP -= 2;
	state->memory[state->SP] = ((state->PC+2) & 0xFF00) >> 8;
	state->memory[state->SP+1] = (state->PC+2) & 0xFF;
	state->PC = ((code[0]&0xf)<<8) | code[1];
}

static void Op3(Chip8State *state, uint8_t *code)
{
	uint8_t reg = code[0] & 0xf;
	if (state->V[reg] == code[1])
		state->PC+=2;
	state->PC+=2;
}

static void Op4(Chip8State *state, uint8_t *code)
{
	uint8_t reg = code[0] & 0xf;
	if (state->V[reg] != code[1])
		state->PC+=2;
	state->PC+=2;
}

static void Op5(Chip8State *state, uint8_t *code)
{
	uint8_t reg1 = code[0] & 0xf;
	uint8_t reg2 = (code[1] & 0xf0)>>4;
	if (state->V[reg1] == state->V[reg2])
		state->PC+=2;
	state->PC+=2;
}

static void Op6(Chip8State *state, uint8_t *code)
{
	uint8_t reg = code[0] & 0xf;
	state->V[reg] = code[1];
	state->PC+=2;
}

static void Op7(Chip8State *state, uint8_t *code)
{
	uint8_t reg = code[0] & 0xf;
	state->V[reg] += code[1];
	state->PC+=2;
}

static void Op8(Chip8State *state, uint8_t *code)
{
	int lastnib = code[1] & 0xf;
	uint8_t vx = code[0] & 0xf;
	uint8_t vy = (code[1] & 0xf0)>>4;

	switch(lastnib)
	{
		case 0: state->V[vx] = state->V[vy]; break;
		case 1: state->V[vx] |= state->V[vy]; break;
		case 2: state->V[vx] &= state->V[vy]; break;
		case 3: state->V[vx] ^= state->V[vy]; break;
		case 4: 
			{
				uint16_t res = state->V[vx] + state->V[vy];
				if (res & 0xff00)
					state->V[0xf] = 1;
				else
					state->V[0xf] = 0;
				state->V[vx] = res&0xff;
			}
			break;
		case 5: 
			{
				int borrow = (state->V[vx] > state->V[vy]);
				state->V[vx] -= state->V[vy];
				state->V[0xf] = borrow;
			}
			break;
		case 6:					//SHR
			{
				uint8_t vf = state->V[vx] & 0x1;
				state->V[vx] = state->V[vx] >> 1;
				state->V[0xf] = vf;
			}
			break; 
		case 7: 
			{
				int borrow = (state->V[vy] > state->V[vx]);
				state->V[vx] = state->V[vy] - state->V[vx];
				state->V[0xf] = borrow;
			}
			break;
		case 0xe:				//SHL
			{
				uint8_t vf = (0x80 == (state->V[vx] & 0x80));
				state->V[vx] = state->V[vx] << 1;
				state->V[0xf] = vf;
			}
		default:
			UnimplementedInstruction(state); 
			break;
	}
	state->PC+=2;
}

static void Op9(Chip8State *state, uint8_t *code)
{
	uint8_t reg1 = code[0] & 0xf;
	uint8_t reg2 = (code[1] & 0xf0)>>4;
	if (state->V[reg1] != state->V[reg2])
		state->PC+=2;
	state->PC+=2;
}

static void OpA(Chip8State *state, uint8_t *code)
{
	state->I = ((code[0] & 0xf)<<8) | code[1];
	state->PC+=2;
}

static void OpB(Chip8State *state, uint8_t *code)
{
	state->PC = ((uint16_t)state->V[0] + (((code[0] & 0xf)<<8) | code[1]));
}

static void OpC(Chip8State *state, uint8_t *code)
{
	uint8_t reg = code[0] & 0xf;
	state->V[reg] = random() & code[1];	
	state->PC+=2;
}

static void OpD(Chip8State *state, uint8_t *code)
{
    
	//Draw sprite
	int lines = code[1]&0xf;
	int x = state->V[code[0] & 0xf];
	int y = state->V[(code[1] & 0xf0) >> 4];	
	int i,j;
	
    state->V[0xf] = 0;
    for (i=0; i<lines; i++)
	{
        uint8_t *sprite = &state->memory[state->I+i];
        int spritebit=7;
        for (j=x; j<(x+8) && j<64; j++)
        {
            int jover8 = j / 8;     //picks the byte in the row
            int jmod8 = j % 8;      //picks the bit in the byte
            uint8_t srcbit = (*sprite >> spritebit) & 0x1;
            
            if (srcbit)
            {
                uint8_t *destbyte_p = &state->screen[ (i+y) * (64/8) + jover8];
                uint8_t destbyte = *destbyte_p;
                uint8_t destmask = (0x80 >> jmod8);
                uint8_t destbit = destbyte & destmask;

                srcbit = srcbit << (7-jmod8);
                
                if (srcbit & destbit)
                    state->V[0xf] = 1;
                
                destbit ^= srcbit;
                
                destbyte = (destbyte & ~destmask) | destbit;

                *destbyte_p = destbyte;
            }
            spritebit--;
        }
    }
	
    
	state->PC+=2;
}

static void OpE(Chip8State *state, uint8_t *code)
{
	int reg = code[0]&0xf;
	switch (code[1])
	{
		case 0x9e:
			if (state->key_state[state->V[reg]] != 0)
				state->PC+=2;
			break;
		case 0xa1:
			//Skips the next instruction if the key stored in VX isn't pressed.
			if (state->key_state[state->V[reg]] == 0)
				state->PC+=2;
			break;
		default:
			UnimplementedInstruction(state); 
			break;
	}
	state->PC+=2;
}

static void OpF(Chip8State *state, uint8_t *code)
{
	int reg = code[0]&0xf;
	switch (code[1])
	{
		case 0x07: state->V[reg] = state->delay; break;
		case 0x0A:
            {
                if (state->waiting_for_key == 0)
                {
                    //start the key_wait_cycle
                    memcpy(&state->save_key_state, &state->key_state, 16);
                    state->waiting_for_key = 1;
                    //do not advance PC
                    return;
                }
                else
                {
                    int i;
                    //look for a change in key_state from 0 to 1
                    for (i = 0; i < 16; i++)
                    {
                        if ((state->save_key_state[i] == 0) && (state->key_state[i] == 1))
                        {
                           state->waiting_for_key = 0;
                           state->V[reg] = i;
                           state->PC += 2;
                           return;
                        }
                        //by copying the key state, this allows detection of a key that 
                        // started pressed, then got released, then was pressed again
                        state->save_key_state[i] = state->key_state[i];
                    }
                    //do not advance the PC
                    return;
                }
            }
            break;
		case 0x15: state->delay = state->V[reg]; break;
		case 0x18: state->sound = state->V[reg]; break;
		case 0x1E: state->I += state->V[reg]; break;
		case 0x29: 
            state->I = FONT_BASE+(state->V[reg] * 5);
            break;
        case 0x33:              //BCD MOV
            {
                uint8_t ones, tens, hundreds;
                uint8_t value=state->V[reg];
                ones = value % 10;
                value = value / 10;
                tens = value % 10;
                hundreds = value / 10;
                state->memory[state->I] = hundreds;
                state->memory[state->I+1] = tens;
                state->memory[state->I+2] = ones;
            }
            break;
		case 0x55:
			{
				int i;
				uint8_t reg = code[0]&0xf;
				for (i=0; i<=reg; i++)
					state->memory[state->I+i] = state->V[i];
				state->I += (reg+1);
			}
			break;
		case 0x65:
			{
				int i;
				uint8_t reg = code[0]&0xf;
				for (i=0; i<=reg; i++)
					state->V[i] = state->memory[state->I+i];
				state->I += (reg+1);
			}
			break;
		default:
			UnimplementedInstruction(state); 
			break;
	}
	state->PC+=2;
}


Chip8State* InitChip8(void)
{
	Chip8State* s = calloc(sizeof(Chip8State), 1);
    
	s->memory = calloc(1024*4, 1);
	s->screen = &s->memory[0xf00];
	s->SP = 0xfa0;
	s->PC = 0x200;
    
    //Put font into lowmem
    memcpy(&s->memory[FONT_BASE], font4x5, FONT_SIZE);
    
	return s;
}

void EmulateChip8Op(Chip8State *state)
{
    uint8_t *op = &state->memory[state->PC];
    DisassembleChip8Op(state->memory, state->PC);
    printf("\n");
    int highnib = (*op & 0xf0) >> 4;
    switch (highnib)
    {
        case 0x00: Op0(state, op); break;
        case 0x01: Op1(state, op); break;
        case 0x02: Op2(state, op); break;
        case 0x03: Op3(state, op); break;
        case 0x04: Op4(state, op); break;
        case 0x05: Op5(state, op); break;
        case 0x06: Op6(state, op); break;
        case 0x07: Op7(state, op); break;
        case 0x08: Op8(state, op); break;
        case 0x09: Op9(state, op); break;
        case 0x0a: OpA(state, op); break;
        case 0x0b: OpB(state, op); break;
        case 0x0c: OpC(state, op); break;
        case 0x0d: OpD(state, op); break;
        case 0x0e: OpE(state, op); break;
        case 0x0f: OpF(state, op); break;
    }
}
