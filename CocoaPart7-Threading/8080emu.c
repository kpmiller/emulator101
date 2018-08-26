/*
 This is free and unencumbered software released into the public domain.
 
 Anyone is free to copy, modify, publish, use, compile, sell, or
 distribute this software, either in source code form or as a compiled
 binary, for any purpose, commercial or non-commercial, and by any
 means.
 
 In jurisdictions that recognize copyright laws, the author or authors
 of this software dedicate any and all copyright interest in the
 software to the public domain. We make this dedication for the benefit
 of the public at large and to the detriment of our heirs and
 successors. We intend this dedication to be an overt act of
 relinquishment in perpetuity of all present and future rights to this
 software under copyright law.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
 
 For more information, please refer to <http://unlicense.org/>
 */

#include "8080emu.h"

#define PRINTOPS 0

static int parity(int x, int size)
{
	int i;
	int p = 0;
	x = (x & ((1<<size)-1));
	for (i=0; i<size; i++)
	{
		if (x & 0x1) p++;
		x = x >> 1;
	}
	return (0 == (p & 0x1));
}


unsigned char cycles8080[] = {
	4, 10, 7, 5, 5, 5, 7, 4, 4, 10, 7, 5, 5, 5, 7, 4, //0x00..0x0f
	4, 10, 7, 5, 5, 5, 7, 4, 4, 10, 7, 5, 5, 5, 7, 4, //0x10..0x1f
	4, 10, 16, 5, 5, 5, 7, 4, 4, 10, 16, 5, 5, 5, 7, 4, //etc
	4, 10, 13, 5, 10, 10, 10, 4, 4, 10, 13, 5, 5, 5, 7, 4,
	
	5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5, //0x40..0x4f
	5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
	5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
	7, 7, 7, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 5, 7, 5,
	
	4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4, //0x80..8x4f
	4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	
	11, 10, 10, 10, 17, 11, 7, 11, 11, 10, 10, 10, 10, 17, 7, 11, //0xc0..0xcf
	11, 10, 10, 10, 17, 11, 7, 11, 11, 10, 10, 10, 10, 17, 7, 11, 
	11, 10, 10, 18, 17, 11, 7, 11, 11, 5, 10, 5, 17, 17, 7, 11, 
	11, 10, 10, 4, 17, 11, 7, 11, 11, 5, 10, 4, 17, 17, 7, 11, 
};

static int Disassemble8080Op(unsigned char *codebuffer, int pc)
{
	unsigned char *code = &codebuffer[pc];
	int opbytes = 1;
	printf("%04x ", pc);
	switch (*code)
	{
		case 0x00: printf("NOP"); break;
		case 0x01: printf("LXI    B,#$%02x%02x", code[2], code[1]); opbytes=3; break;
		case 0x02: printf("STAX   B"); break;
		case 0x03: printf("INX    B"); break;
		case 0x04: printf("INR    B"); break;
		case 0x05: printf("DCR    B"); break;
		case 0x06: printf("MVI    B,#$%02x", code[1]); opbytes=2; break;
		case 0x07: printf("RLC"); break;
		case 0x08: printf("NOP"); break;
		case 0x09: printf("DAD    B"); break;
		case 0x0a: printf("LDAX   B"); break;
		case 0x0b: printf("DCX    B"); break;
		case 0x0c: printf("INR    C"); break;
		case 0x0d: printf("DCR    C"); break;
		case 0x0e: printf("MVI    C,#$%02x", code[1]); opbytes = 2;	break;
		case 0x0f: printf("RRC"); break;
			
		case 0x10: printf("NOP"); break;
		case 0x11: printf("LXI    D,#$%02x%02x", code[2], code[1]); opbytes=3; break;
		case 0x12: printf("STAX   D"); break;
		case 0x13: printf("INX    D"); break;
		case 0x14: printf("INR    D"); break;
		case 0x15: printf("DCR    D"); break;
		case 0x16: printf("MVI    D,#$%02x", code[1]); opbytes=2; break;
		case 0x17: printf("RAL"); break;
		case 0x18: printf("NOP"); break;
		case 0x19: printf("DAD    D"); break;
		case 0x1a: printf("LDAX   D"); break;
		case 0x1b: printf("DCX    D"); break;
		case 0x1c: printf("INR    E"); break;
		case 0x1d: printf("DCR    E"); break;
		case 0x1e: printf("MVI    E,#$%02x", code[1]); opbytes = 2; break;
		case 0x1f: printf("RAR"); break;
			
		case 0x20: printf("NOP"); break;
		case 0x21: printf("LXI    H,#$%02x%02x", code[2], code[1]); opbytes=3; break;
		case 0x22: printf("SHLD   $%02x%02x", code[2], code[1]); opbytes=3; break;
		case 0x23: printf("INX    H"); break;
		case 0x24: printf("INR    H"); break;
		case 0x25: printf("DCR    H"); break;
		case 0x26: printf("MVI    H,#$%02x", code[1]); opbytes=2; break;
		case 0x27: printf("DAA"); break;
		case 0x28: printf("NOP"); break;
		case 0x29: printf("DAD    H"); break;
		case 0x2a: printf("LHLD   $%02x%02x", code[2], code[1]); opbytes=3; break;
		case 0x2b: printf("DCX    H"); break;
		case 0x2c: printf("INR    L"); break;
		case 0x2d: printf("DCR    L"); break;
		case 0x2e: printf("MVI    L,#$%02x", code[1]); opbytes = 2; break;
		case 0x2f: printf("CMA"); break;
			
		case 0x30: printf("NOP"); break;
		case 0x31: printf("LXI    SP,#$%02x%02x", code[2], code[1]); opbytes=3; break;
		case 0x32: printf("STA    $%02x%02x", code[2], code[1]); opbytes=3; break;
		case 0x33: printf("INX    SP"); break;
		case 0x34: printf("INR    M"); break;
		case 0x35: printf("DCR    M"); break;
		case 0x36: printf("MVI    M,#$%02x", code[1]); opbytes=2; break;
		case 0x37: printf("STC"); break;
		case 0x38: printf("NOP"); break;
		case 0x39: printf("DAD    SP"); break;
		case 0x3a: printf("LDA    $%02x%02x", code[2], code[1]); opbytes=3; break;
		case 0x3b: printf("DCX    SP"); break;
		case 0x3c: printf("INR    A"); break;
		case 0x3d: printf("DCR    A"); break;
		case 0x3e: printf("MVI    A,#$%02x", code[1]); opbytes = 2; break;
		case 0x3f: printf("CMC"); break;
			
		case 0x40: printf("MOV    B,B"); break;
		case 0x41: printf("MOV    B,C"); break;
		case 0x42: printf("MOV    B,D"); break;
		case 0x43: printf("MOV    B,E"); break;
		case 0x44: printf("MOV    B,H"); break;
		case 0x45: printf("MOV    B,L"); break;
		case 0x46: printf("MOV    B,M"); break;
		case 0x47: printf("MOV    B,A"); break;
		case 0x48: printf("MOV    C,B"); break;
		case 0x49: printf("MOV    C,C"); break;
		case 0x4a: printf("MOV    C,D"); break;
		case 0x4b: printf("MOV    C,E"); break;
		case 0x4c: printf("MOV    C,H"); break;
		case 0x4d: printf("MOV    C,L"); break;
		case 0x4e: printf("MOV    C,M"); break;
		case 0x4f: printf("MOV    C,A"); break;
			
		case 0x50: printf("MOV    D,B"); break;
		case 0x51: printf("MOV    D,C"); break;
		case 0x52: printf("MOV    D,D"); break;
		case 0x53: printf("MOV    D.E"); break;
		case 0x54: printf("MOV    D,H"); break;
		case 0x55: printf("MOV    D,L"); break;
		case 0x56: printf("MOV    D,M"); break;
		case 0x57: printf("MOV    D,A"); break;
		case 0x58: printf("MOV    E,B"); break;
		case 0x59: printf("MOV    E,C"); break;
		case 0x5a: printf("MOV    E,D"); break;
		case 0x5b: printf("MOV    E,E"); break;
		case 0x5c: printf("MOV    E,H"); break;
		case 0x5d: printf("MOV    E,L"); break;
		case 0x5e: printf("MOV    E,M"); break;
		case 0x5f: printf("MOV    E,A"); break;
            
		case 0x60: printf("MOV    H,B"); break;
		case 0x61: printf("MOV    H,C"); break;
		case 0x62: printf("MOV    H,D"); break;
		case 0x63: printf("MOV    H.E"); break;
		case 0x64: printf("MOV    H,H"); break;
		case 0x65: printf("MOV    H,L"); break;
		case 0x66: printf("MOV    H,M"); break;
		case 0x67: printf("MOV    H,A"); break;
		case 0x68: printf("MOV    L,B"); break;
		case 0x69: printf("MOV    L,C"); break;
		case 0x6a: printf("MOV    L,D"); break;
		case 0x6b: printf("MOV    L,E"); break;
		case 0x6c: printf("MOV    L,H"); break;
		case 0x6d: printf("MOV    L,L"); break;
		case 0x6e: printf("MOV    L,M"); break;
		case 0x6f: printf("MOV    L,A"); break;
            
		case 0x70: printf("MOV    M,B"); break;
		case 0x71: printf("MOV    M,C"); break;
		case 0x72: printf("MOV    M,D"); break;
		case 0x73: printf("MOV    M.E"); break;
		case 0x74: printf("MOV    M,H"); break;
		case 0x75: printf("MOV    M,L"); break;
		case 0x76: printf("HLT");        break;
		case 0x77: printf("MOV    M,A"); break;
		case 0x78: printf("MOV    A,B"); break;
		case 0x79: printf("MOV    A,C"); break;
		case 0x7a: printf("MOV    A,D"); break;
		case 0x7b: printf("MOV    A,E"); break;
		case 0x7c: printf("MOV    A,H"); break;
		case 0x7d: printf("MOV    A,L"); break;
		case 0x7e: printf("MOV    A,M"); break;
		case 0x7f: printf("MOV    A,A"); break;
            
		case 0x80: printf("ADD    B"); break;
		case 0x81: printf("ADD    C"); break;
		case 0x82: printf("ADD    D"); break;
		case 0x83: printf("ADD    E"); break;
		case 0x84: printf("ADD    H"); break;
		case 0x85: printf("ADD    L"); break;
		case 0x86: printf("ADD    M"); break;
		case 0x87: printf("ADD    A"); break;
		case 0x88: printf("ADC    B"); break;
		case 0x89: printf("ADC    C"); break;
		case 0x8a: printf("ADC    D"); break;
		case 0x8b: printf("ADC    E"); break;
		case 0x8c: printf("ADC    H"); break;
		case 0x8d: printf("ADC    L"); break;
		case 0x8e: printf("ADC    M"); break;
		case 0x8f: printf("ADC    A"); break;
            
		case 0x90: printf("SUB    B"); break;
		case 0x91: printf("SUB    C"); break;
		case 0x92: printf("SUB    D"); break;
		case 0x93: printf("SUB    E"); break;
		case 0x94: printf("SUB    H"); break;
		case 0x95: printf("SUB    L"); break;
		case 0x96: printf("SUB    M"); break;
		case 0x97: printf("SUB    A"); break;
		case 0x98: printf("SBB    B"); break;
		case 0x99: printf("SBB    C"); break;
		case 0x9a: printf("SBB    D"); break;
		case 0x9b: printf("SBB    E"); break;
		case 0x9c: printf("SBB    H"); break;
		case 0x9d: printf("SBB    L"); break;
		case 0x9e: printf("SBB    M"); break;
		case 0x9f: printf("SBB    A"); break;
            
		case 0xa0: printf("ANA    B"); break;
		case 0xa1: printf("ANA    C"); break;
		case 0xa2: printf("ANA    D"); break;
		case 0xa3: printf("ANA    E"); break;
		case 0xa4: printf("ANA    H"); break;
		case 0xa5: printf("ANA    L"); break;
		case 0xa6: printf("ANA    M"); break;
		case 0xa7: printf("ANA    A"); break;
		case 0xa8: printf("XRA    B"); break;
		case 0xa9: printf("XRA    C"); break;
		case 0xaa: printf("XRA    D"); break;
		case 0xab: printf("XRA    E"); break;
		case 0xac: printf("XRA    H"); break;
		case 0xad: printf("XRA    L"); break;
		case 0xae: printf("XRA    M"); break;
		case 0xaf: printf("XRA    A"); break;
            
		case 0xb0: printf("ORA    B"); break;
		case 0xb1: printf("ORA    C"); break;
		case 0xb2: printf("ORA    D"); break;
		case 0xb3: printf("ORA    E"); break;
		case 0xb4: printf("ORA    H"); break;
		case 0xb5: printf("ORA    L"); break;
		case 0xb6: printf("ORA    M"); break;
		case 0xb7: printf("ORA    A"); break;
		case 0xb8: printf("CMP    B"); break;
		case 0xb9: printf("CMP    C"); break;
		case 0xba: printf("CMP    D"); break;
		case 0xbb: printf("CMP    E"); break;
		case 0xbc: printf("CMP    H"); break;
		case 0xbd: printf("CMP    L"); break;
		case 0xbe: printf("CMP    M"); break;
		case 0xbf: printf("CMP    A"); break;
            
		case 0xc0: printf("RNZ"); break;
		case 0xc1: printf("POP    B"); break;
		case 0xc2: printf("JNZ    $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xc3: printf("JMP    $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xc4: printf("CNZ    $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xc5: printf("PUSH   B"); break;
		case 0xc6: printf("ADI    #$%02x",code[1]); opbytes = 2; break;
		case 0xc7: printf("RST    0"); break;
		case 0xc8: printf("RZ"); break;
		case 0xc9: printf("RET"); break;
		case 0xca: printf("JZ     $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xcb: printf("JMP    $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xcc: printf("CZ     $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xcd: printf("CALL   $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xce: printf("ACI    #$%02x",code[1]); opbytes = 2; break;
		case 0xcf: printf("RST    1"); break;
            
		case 0xd0: printf("RNC"); break;
		case 0xd1: printf("POP    D"); break;
		case 0xd2: printf("JNC    $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xd3: printf("OUT    #$%02x",code[1]); opbytes = 2; break;
		case 0xd4: printf("CNC    $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xd5: printf("PUSH   D"); break;
		case 0xd6: printf("SUI    #$%02x",code[1]); opbytes = 2; break;
		case 0xd7: printf("RST    2"); break;
		case 0xd8: printf("RC");  break;
		case 0xd9: printf("RET"); break;
		case 0xda: printf("JC     $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xdb: printf("IN     #$%02x",code[1]); opbytes = 2; break;
		case 0xdc: printf("CC     $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xdd: printf("CALL   $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xde: printf("SBI    #$%02x",code[1]); opbytes = 2; break;
		case 0xdf: printf("RST    3"); break;
            
		case 0xe0: printf("RPO"); break;
		case 0xe1: printf("POP    H"); break;
		case 0xe2: printf("JPO    $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xe3: printf("XTHL");break;
		case 0xe4: printf("CPO    $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xe5: printf("PUSH   H"); break;
		case 0xe6: printf("ANI    #$%02x",code[1]); opbytes = 2; break;
		case 0xe7: printf("RST    4"); break;
		case 0xe8: printf("RPE"); break;
		case 0xe9: printf("PCHL");break;
		case 0xea: printf("JPE    $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xeb: printf("XCHG"); break;
		case 0xec: printf("CPE     $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xed: printf("CALL   $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xee: printf("XRI    #$%02x",code[1]); opbytes = 2; break;
		case 0xef: printf("RST    5"); break;
            
		case 0xf0: printf("RP");  break;
		case 0xf1: printf("POP    PSW"); break;
		case 0xf2: printf("JP     $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xf3: printf("DI");  break;
		case 0xf4: printf("CP     $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xf5: printf("PUSH   PSW"); break;
		case 0xf6: printf("ORI    #$%02x",code[1]); opbytes = 2; break;
		case 0xf7: printf("RST    6"); break;
		case 0xf8: printf("RM");  break;
		case 0xf9: printf("SPHL");break;
		case 0xfa: printf("JM     $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xfb: printf("EI");  break;
		case 0xfc: printf("CM     $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xfd: printf("CALL   $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xfe: printf("CPI    #$%02x",code[1]); opbytes = 2; break;
		case 0xff: printf("RST    7"); break;
	}
	
	return opbytes;
}

static void LogicFlagsA(State8080 *state)
{
	state->cc.cy = state->cc.ac = 0;
	state->cc.z = (state->a == 0);
	state->cc.s = (0x80 == (state->a & 0x80));
	state->cc.p = parity(state->a, 8);
}

static void ArithFlagsA(State8080 *state, uint16_t res)
{
	state->cc.cy = (res > 0xff);
	state->cc.z = ((res&0xff) == 0);
	state->cc.s = (0x80 == (res & 0x80));
	state->cc.p = parity(res&0xff, 8);
}

static void UnimplementedInstruction(State8080* state)
{
	//pc will have advanced one, so undo that
	printf ("Error: Unimplemented instruction\n");
	state->pc--;
	Disassemble8080Op(state->memory, state->pc);
	printf("\n");
	exit(1);
}

static void WriteMem(State8080* state, uint16_t address, uint8_t value)
{
    if (address < 0x2000)
    {
        //        printf("Writing ROM not allowed %x\n", address);
        return;
    }
    if (address >=0x4000)
    {
        //       printf("Writing out of Space Invaders RAM not allowed %x\n", address);
        return;
    }
    
    state->memory[address] = value;
}

static uint8_t ReadFromHL(State8080* state)
{
    uint16_t offset = (state->h << 8) | state->l;
    return state->memory[offset];
}

static void WriteToHL(State8080* state, uint8_t value)
{
    uint16_t offset = (state->h << 8) | state->l;
    WriteMem(state, offset, value);
}

static void Push(State8080* state, uint8_t high, uint8_t low)
{
    WriteMem(state, state->sp-1, high);
    WriteMem(state, state->sp-2, low);
    state->sp = state->sp - 2;    
    //    printf ("%04x %04x\n", state->pc, state->sp);
}

static void Pop(State8080* state, uint8_t *high, uint8_t *low)
{
    *low = state->memory[state->sp];
    *high = state->memory[state->sp+1];
    state->sp += 2;    
    //    printf ("%04x %04x pop\n", state->pc, state->sp);
}

static void FlagsZSP(State8080 *state, uint8_t value)
{
    state->cc.z = (value == 0);
    state->cc.s = (0x80 == (value & 0x80));
    state->cc.p = parity(value, 8);    
}

int Emulate8080Op(State8080* state)
{
	unsigned char *opcode = &state->memory[state->pc];
    
#if PRINTOPS
    Disassemble8080Op(state->memory, state->pc);
#endif

	state->pc+=1;    
    
	switch (*opcode)
	{
		case 0x00: break;	//NOP
		case 0x01: 							//LXI	B,word
			state->c = opcode[1];
			state->b = opcode[2];
			state->pc += 2;
			break;
		case 0x02: 							//STAX B
        {
            uint16_t offset=(state->b<<8) | state->c;
            WriteMem(state, offset, state->a);
        }
			break;
		case 0x03: 							//INX    B
			state->c++;
			if (state->c == 0)
				state->b++;
			break;
		case 0x04: 							//INR	B
            state->b += 1;
            FlagsZSP(state,state->b);
			break;		
		case 0x05: 							//DCR    B
			state->b -= 1;
            FlagsZSP(state,state->b);
			break;
		case 0x06: 							//MVI B,byte
			state->b = opcode[1];
			state->pc++;
			break;
		case 0x07: 							//RLC
        {
            uint8_t x = state->a;
            state->a = ((x & 0x80) >> 7) | (x << 1);
            state->cc.cy = (0x80 == (x&0x80));
        }
			break;
		case 0x08: UnimplementedInstruction(state); break;
		case 0x09: 							//DAD B
        {
			uint32_t hl = (state->h << 8) | state->l;
			uint32_t bc = (state->b << 8) | state->c;
			uint32_t res = hl + bc;
			state->h = (res & 0xff00) >> 8;
			state->l = res & 0xff;
			state->cc.cy = ((res & 0xffff0000) != 0);
        }
			break;
		case 0x0a: 							//LDAX   B
        {
			uint16_t offset=(state->b<<8) | state->c;
			state->a = state->memory[offset];
        }
			break;
		case 0x0b:							//DCX B 
			state->c -= 1;
			if (state->c==0xff)
				state->b-=1;
			break;
		case 0x0c:                          //INR C
            state->c += 1;
            FlagsZSP(state,state->c);
			break;		
            
		case 0x0d: 							//DCR C
            state->c -= 1;
            FlagsZSP(state,state->c);
			break;
		case 0x0e: 							//MVI C,byte
			state->c = opcode[1];
			state->pc++;
			break;
		case 0x0f: 							//RRC
        {
            uint8_t x = state->a;
            state->a = ((x & 1) << 7) | (x >> 1);
            state->cc.cy = (1 == (x&1));
        }
			break;
			
		case 0x10: UnimplementedInstruction(state); break;
		case 0x11: 							//LXI	D,word
			state->e = opcode[1];
			state->d = opcode[2];
			state->pc += 2;
			break;
		case 0x12: 							//STAX D
        {
            uint16_t offset=(state->d<<8) | state->e;
            WriteMem(state, offset, state->a);
        }
			break;
		case 0x13: 							//INX    D
			state->e++;
			if (state->e == 0)
				state->d++;
			break;		
		case 0x14: 							//INR D
            state->d += 1;
            FlagsZSP(state,state->d);
			break;		
		case 0x15: 							//DCR    D
            state->d -= 1;
            FlagsZSP(state,state->d);
			break;
		case 0x16: 							//MVI D,byte
			state->d = opcode[1];
			state->pc++;
			break;
		case 0x17:                           //RAL
        {
            uint8_t x = state->a;
            state->a = state->cc.cy  | (x << 1);
            state->cc.cy = (0x80 == (x&0x80));
        }
			break;			
		case 0x18: UnimplementedInstruction(state); break;
		case 0x19: 							//DAD    D
        {
			uint32_t hl = (state->h << 8) | state->l;
			uint32_t de = (state->d << 8) | state->e;
			uint32_t res = hl + de;
			state->h = (res & 0xff00) >> 8;
			state->l = res & 0xff;
			state->cc.cy = ((res & 0xffff0000) != 0);
        }
			break;
		case 0x1a: 							//LDAX	D
        {
			uint16_t offset=(state->d<<8) | state->e;
			state->a = state->memory[offset];
        }
			break;
		case 0x1b: 							//DCX D
			state->e -= 1;
			if (state->e==0xff)
				state->d-=1;
			break;
		case 0x1c:							//INR E
            state->e += 1;
            FlagsZSP(state,state->e);
			break;		
            
		case 0x1d: 							//DCR    E
            state->e -= 1;
            FlagsZSP(state,state->e);
			break;
		case 0x1e: 							//MVI E,byte
			state->e = opcode[1];
			state->pc++;
			break;
		case 0x1f:                           //RAR
        {
            uint8_t x = state->a;
            state->a = (state->cc.cy << 7) | (x >> 1);
            state->cc.cy = (1 == (x&1));
        }
			break;			
		case 0x20: UnimplementedInstruction(state); break;
		case 0x21: 							//LXI	H,word
			state->l = opcode[1];
			state->h = opcode[2];
			state->pc += 2;
			break;
		case 0x22: 							//SHLD
        {
            uint16_t offset = opcode[1] | (opcode[2] << 8);
            WriteMem(state, offset, state->l);
            WriteMem(state, offset+1, state->h);
            state->pc += 2;
        }
			break;
		case 0x23: 							//INX    H
			state->l++;
			if (state->l == 0)
				state->h++;
			break;		
		case 0x24:							//INR	H 
            state->h += 1;
            FlagsZSP(state,state->h);
			break;		
		case 0x25: 							//DCR    H
            state->h -= 1;
            FlagsZSP(state,state->h);
			break;
		case 0x26:  							//MVI H,byte
			state->h = opcode[1];
			state->pc++;
			break;
		case 0x27: 
            if ((state->a &0xf) > 9)
                state->a += 6;
            if ((state->a&0xf0) > 0x90)
            {
                uint16_t res = (uint16_t) state->a + 0x60;
                state->a = res & 0xff;
                ArithFlagsA(state, res);
            }
            break;
		case 0x28: UnimplementedInstruction(state); break;
		case 0x29: 								//DAD    H
        {
			uint32_t hl = (state->h << 8) | state->l;
			uint32_t res = hl + hl;
			state->h = (res & 0xff00) >> 8;
			state->l = res & 0xff;
			state->cc.cy = ((res & 0xffff0000) != 0);
        }
			break;
		case 0x2a: 								//LHLD adr
        {
            uint16_t offset = opcode[1] | (opcode[2] << 8);
            state->l = state->memory[offset];
            state->h = state->memory[offset+1];
            state->pc += 2;
        }
			break;
		case 0x2b: 								//DCX H
			state->l -= 1;
			if (state->l==0xff)
				state->h-=1;
			break;		
		case 0x2c:								//INR L
            state->l += 1;
            FlagsZSP(state,state->l);
			break;		
		case 0x2d: 							//DCR    L
            state->l -= 1;
            FlagsZSP(state,state->l);
			break;
		case 0x2e:  						//MVI L,byte
			state->l = opcode[1];
			state->pc++;
			break;
		case 0x2f:
			state->a = ~state->a;			//CMA 
			break;
		case 0x30: UnimplementedInstruction(state); break;
		case 0x31: 							//LXI	SP,word
			state->sp = (opcode[2]<<8) | opcode[1];
			state->pc += 2;
			break;
		case 0x32: 							//STA    (word)
        {
			uint16_t offset = (opcode[2]<<8) | (opcode[1]);
            WriteMem(state, offset, state->a);
			state->pc += 2;
        }
			break;
		case 0x33: 							//INX    SP
			state->sp++;
			break;		
		case 0x34: 							//INR	M
        {
			uint8_t res = ReadFromHL(state) + 1;
            FlagsZSP(state, res);
            WriteToHL(state, res);
        }
			break;			
		case 0x35: 							//DCR    M
        {
			uint8_t res = ReadFromHL(state) - 1;
            FlagsZSP(state, res);
            WriteToHL(state, res);
        }
			break;			
		case 0x36: 							//MVI	M,byte
        {					
            WriteToHL(state, opcode[1]);
			state->pc++;
        }
			break;
		case 0x37: state->cc.cy = 1; break;
		case 0x38: UnimplementedInstruction(state); break;
		case 0x39: 							//DAD    SP
        {
			uint32_t hl = (state->h << 8) | state->l;
			uint32_t res = hl + state->sp;
			state->h = (res & 0xff00) >> 8;
			state->l = res & 0xff;
			state->cc.cy = ((res & 0xffff0000) > 0);
        }
			break;
		case 0x3a: 							//LDA    (word)
        {
			uint16_t offset = (opcode[2]<<8) | (opcode[1]);
			state->a = state->memory[offset];
			state->pc+=2;
        }
			break;
		case 0x3b: 							//DCX SP
			state->sp -= 1;
			break;
		case 0x3c: 							//INR A
            state->a += 1;
            FlagsZSP(state,state->a);
			break;		
		case 0x3d:							//DCR A
            state->a -= 1;
            FlagsZSP(state,state->a);
			break;
		case 0x3e: 							//MVI    A,byte
			state->a = opcode[1];
			state->pc++;
			break;
		case 0x3f: state->cc.cy = 0; break;
			
		case 0x40: state->b = state->b; break;
		case 0x41: state->b = state->c; break;
		case 0x42: state->b = state->d; break;
		case 0x43: state->b = state->e; break;
		case 0x44: state->b = state->h; break;
		case 0x45: state->b = state->l; break;
		case 0x46: state->b = ReadFromHL(state); break;
		case 0x47: state->b = state->a; break;
            
		case 0x48: state->c = state->b; break;
		case 0x49: state->c = state->c; break;
		case 0x4a: state->c = state->d; break;
		case 0x4b: state->c = state->e; break;
		case 0x4c: state->c = state->h; break;
		case 0x4d: state->c = state->l; break;
		case 0x4e: state->c = ReadFromHL(state); break;
		case 0x4f: state->c = state->a; break;
			
		case 0x50: state->d = state->b; break;
		case 0x51: state->d = state->c; break;
		case 0x52: state->d = state->d; break;
		case 0x53: state->d = state->e; break;
		case 0x54: state->d = state->h; break;
		case 0x55: state->d = state->l; break;
		case 0x56: state->d = ReadFromHL(state); break;
		case 0x57: state->d = state->a; break;
            
		case 0x58: state->e = state->b; break;
		case 0x59: state->e = state->c; break;
		case 0x5a: state->e = state->d; break;
		case 0x5b: state->e = state->e; break;
		case 0x5c: state->e = state->h; break;
		case 0x5d: state->e = state->l; break;
		case 0x5e: state->e = ReadFromHL(state); break;
		case 0x5f: state->e = state->a; break;
            
		case 0x60: state->h = state->b; break;
		case 0x61: state->h = state->c; break;
		case 0x62: state->h = state->d; break;
		case 0x63: state->h = state->e; break;
		case 0x64: state->h = state->h; break;
		case 0x65: state->h = state->l; break;
		case 0x66: state->h = ReadFromHL(state); break;
		case 0x67: state->h = state->a; break;
            
		case 0x68: state->l = state->b; break;
		case 0x69: state->l = state->c; break;
		case 0x6a: state->l = state->d; break;
		case 0x6b: state->l = state->e; break;
		case 0x6c: state->l = state->h; break;
		case 0x6d: state->l = state->l; break;
		case 0x6e: state->l = ReadFromHL(state); break;
		case 0x6f: state->l = state->a; break;
            
		case 0x70: WriteToHL(state, state->b); break;		//MOV    M,B
		case 0x71: WriteToHL(state, state->c); break;		//MOV    M,C
		case 0x72: WriteToHL(state, state->d); break;		//MOV    M,D
		case 0x73: WriteToHL(state, state->e); break;		//MOV    M,E
		case 0x74: WriteToHL(state, state->h); break;		//MOV    M,H
		case 0x75: WriteToHL(state, state->l); break;		//MOV    M,L
		case 0x76:  break;                                  //HLT
		case 0x77: WriteToHL(state, state->a); break;		//MOV    M,A
            
		case 0x78: state->a  = state->b; break;
		case 0x79: state->a  = state->c; break;
		case 0x7a: state->a  = state->d;  break;
		case 0x7b: state->a  = state->e;  break;
		case 0x7c: state->a  = state->h;  break;
		case 0x7d: state->a  = state->l;  break;
		case 0x7e: state->a = ReadFromHL(state); break;
		case 0x7f: break;
            
		case 0x80:  { uint16_t res = (uint16_t) state->a + (uint16_t) state->b; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //ADD B
		case 0x81:  { uint16_t res = (uint16_t) state->a + (uint16_t) state->c; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //ADD C
		case 0x82:  { uint16_t res = (uint16_t) state->a + (uint16_t) state->d; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //ADD D
		case 0x83:  { uint16_t res = (uint16_t) state->a + (uint16_t) state->e; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //ADD E
		case 0x84:  { uint16_t res = (uint16_t) state->a + (uint16_t) state->h; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //ADD H
		case 0x85:  { uint16_t res = (uint16_t) state->a + (uint16_t) state->l; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //ADD L
		case 0x86:  					//ADD M
        { 
            uint16_t res = (uint16_t) state->a + (uint16_t) ReadFromHL(state); 
            ArithFlagsA(state, res); 
            state->a=(res&0xff); 
        } break;  
		case 0x87:  { uint16_t res = (uint16_t) state->a + (uint16_t) state->a; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //ADC A
            
		case 0x88:  { uint16_t res = (uint16_t) state->a + (uint16_t) state->b + state->cc.cy; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //ADC B
		case 0x89:  { uint16_t res = (uint16_t) state->a + (uint16_t) state->c + state->cc.cy; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //ADC C
		case 0x8a:  { uint16_t res = (uint16_t) state->a + (uint16_t) state->d + state->cc.cy; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //ADC D
		case 0x8b:  { uint16_t res = (uint16_t) state->a + (uint16_t) state->e + state->cc.cy; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //ADC E
		case 0x8c:  { uint16_t res = (uint16_t) state->a + (uint16_t) state->h + state->cc.cy; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //ADC H
		case 0x8d:  { uint16_t res = (uint16_t) state->a + (uint16_t) state->l + state->cc.cy; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //ADC L
		case 0x8e: 					//ADC M
        { 
            uint16_t res = (uint16_t) state->a + (uint16_t) ReadFromHL(state) + state->cc.cy; 
            ArithFlagsA(state, res); 
            state->a=(res&0xff); 
        } break;  
		case 0x8f:  { uint16_t res = (uint16_t) state->a + (uint16_t) state->a + state->cc.cy; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //ADC A
            
            
		case 0x90:  { uint16_t res = (uint16_t) state->a - (uint16_t) state->b; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //SUB B
		case 0x91:  { uint16_t res = (uint16_t) state->a - (uint16_t) state->c; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //SUB C
		case 0x92:  { uint16_t res = (uint16_t) state->a - (uint16_t) state->d; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //SUB D
		case 0x93:  { uint16_t res = (uint16_t) state->a - (uint16_t) state->e; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //SUB E
		case 0x94:  { uint16_t res = (uint16_t) state->a - (uint16_t) state->h; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //SUB H
		case 0x95:  { uint16_t res = (uint16_t) state->a - (uint16_t) state->l; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //SUB L
		case 0x96:  					//SUB M
        { 
            uint16_t res = (uint16_t) state->a - (uint16_t) ReadFromHL(state); 
            ArithFlagsA(state, res); 
            state->a=(res&0xff); 
        } break;  
		case 0x97:  { uint16_t res = (uint16_t) state->a - (uint16_t) state->a; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //SUB A
            
		case 0x98:  { uint16_t res = (uint16_t) state->a - (uint16_t) state->b - state->cc.cy; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //SBB B
		case 0x99:  { uint16_t res = (uint16_t) state->a - (uint16_t) state->c - state->cc.cy; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //SBB C
		case 0x9a:  { uint16_t res = (uint16_t) state->a - (uint16_t) state->d - state->cc.cy; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //SBB D
		case 0x9b:  { uint16_t res = (uint16_t) state->a - (uint16_t) state->e - state->cc.cy; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //SBB E
		case 0x9c:  { uint16_t res = (uint16_t) state->a - (uint16_t) state->h - state->cc.cy; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //SBB H
		case 0x9d:  { uint16_t res = (uint16_t) state->a - (uint16_t) state->l - state->cc.cy; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //SBB L
		case 0x9e: 					//SBB M
        { 
            uint16_t res = (uint16_t) state->a - (uint16_t) ReadFromHL(state) - state->cc.cy; 
            ArithFlagsA(state, res); 
            state->a=(res&0xff); 
        } break;  
		case 0x9f:  { uint16_t res = (uint16_t) state->a - (uint16_t) state->a - state->cc.cy; ArithFlagsA(state, res); state->a=(res&0xff); } break;  //SBB A
            
		case 0xa0: state->a = state->a & state->b; LogicFlagsA(state);	break;
		case 0xa1: state->a = state->a & state->c; LogicFlagsA(state);	break;
		case 0xa2: state->a = state->a & state->d; LogicFlagsA(state);	break;
		case 0xa3: state->a = state->a & state->e; LogicFlagsA(state);	break;
		case 0xa4: state->a = state->a & state->h; LogicFlagsA(state);	break;
		case 0xa5: state->a = state->a & state->l; LogicFlagsA(state);	break;
		case 0xa6: state->a = state->a & ReadFromHL(state); LogicFlagsA(state);	break;
		case 0xa7: state->a = state->a & state->a; LogicFlagsA(state);	break;
            
		case 0xa8: state->a = state->a ^ state->b; LogicFlagsA(state);	break;
		case 0xa9: state->a = state->a ^ state->c; LogicFlagsA(state);	break;
		case 0xaa: state->a = state->a ^ state->d; LogicFlagsA(state);	break;
		case 0xab: state->a = state->a ^ state->e; LogicFlagsA(state);	break;
		case 0xac: state->a = state->a ^ state->h; LogicFlagsA(state);	break;
		case 0xad: state->a = state->a ^ state->l; LogicFlagsA(state);	break;
		case 0xae: state->a = state->a ^ ReadFromHL(state); LogicFlagsA(state);	break;
		case 0xaf: state->a = state->a ^ state->a; LogicFlagsA(state);	break;
            
		case 0xb0: state->a = state->a | state->b; LogicFlagsA(state);	break;
		case 0xb1: state->a = state->a | state->c; LogicFlagsA(state);	break;
		case 0xb2: state->a = state->a | state->d; LogicFlagsA(state);	break;
		case 0xb3: state->a = state->a | state->e; LogicFlagsA(state);	break;
		case 0xb4: state->a = state->a | state->h; LogicFlagsA(state);	break;
		case 0xb5: state->a = state->a | state->l; LogicFlagsA(state);	break;
		case 0xb6: state->a = state->a | ReadFromHL(state); LogicFlagsA(state);	break;
		case 0xb7: state->a = state->a | state->a; LogicFlagsA(state);	break;
            
		case 0xb8:  { uint16_t res = (uint16_t) state->a - (uint16_t) state->b; ArithFlagsA(state, res); } break;  //CMP B
		case 0xb9:  { uint16_t res = (uint16_t) state->a - (uint16_t) state->c; ArithFlagsA(state, res); } break;  //CMP C
		case 0xba:  { uint16_t res = (uint16_t) state->a - (uint16_t) state->d; ArithFlagsA(state, res); } break;  //CMP D
		case 0xbb:  { uint16_t res = (uint16_t) state->a - (uint16_t) state->e; ArithFlagsA(state, res); } break;  //CMP E
		case 0xbc:  { uint16_t res = (uint16_t) state->a - (uint16_t) state->h; ArithFlagsA(state, res); } break;  //CMP H
		case 0xbd:  { uint16_t res = (uint16_t) state->a - (uint16_t) state->l; ArithFlagsA(state, res); } break;  //CMP L
		case 0xbe:  { uint16_t res = (uint16_t) state->a - (uint16_t) ReadFromHL(state); ArithFlagsA(state, res); } break;  //CMP L
		case 0xbf:  { uint16_t res = (uint16_t) state->a - (uint16_t) state->a; ArithFlagsA(state, res); } break;  //CMP A
            
		case 0xc0:						//RNZ
			if (state->cc.z == 0)
			{
				state->pc = state->memory[state->sp] | (state->memory[state->sp+1]<<8);
				state->sp += 2;
			}
			break;
            
		case 0xc1: 						//POP    B
            Pop(state, &state->b, &state->c);
			break;
		case 0xc2: 						//JNZ address
			if (0 == state->cc.z)
				state->pc = (opcode[2] << 8) | opcode[1];
			else
				state->pc += 2;
			break;
		case 0xc3:						//JMP address
			state->pc = (opcode[2] << 8) | opcode[1];
			break;
		case 0xc4: 						//CNZ adr
			if (state->cc.z == 0)
			{
				uint16_t	ret = state->pc+2;
                WriteMem(state, state->sp-1, (ret >> 8) & 0xff);
                WriteMem(state, state->sp-2, (ret & 0xff));
				state->sp = state->sp - 2;
				state->pc = (opcode[2] << 8) | opcode[1];
			}
			else
				state->pc += 2;
			break;
            
		case 0xc5: 						//PUSH   B
            Push(state, state->b, state->c);
			break;
		case 0xc6: 						//ADI    byte
        {
			uint16_t x = (uint16_t) state->a + (uint16_t) opcode[1];
            FlagsZSP(state, x&0xff);
			state->cc.cy = (x > 0xff);
			state->a = x&0xff;
			state->pc++;
        }
			break;
		case 0xc7: 					//RST 0
        {
			uint16_t	ret = state->pc+2;
            WriteMem(state, state->sp-1, (ret >> 8) & 0xff);
            WriteMem(state, state->sp-2, (ret & 0xff));
			state->sp = state->sp - 2;
			state->pc = 0x0000;
        }
			break;
		case 0xc8: 					//RZ
			if (state->cc.z)
			{
				state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
				state->sp += 2;
			}
			break;
		case 0xc9: 						//RET
			state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
			state->sp += 2;
			break;
		case 0xca: 						//JZ adr
			if (state->cc.z)
				state->pc = (opcode[2] << 8) | opcode[1];
			else
				state->pc += 2;
			break;
		case 0xcb: UnimplementedInstruction(state); break;
		case 0xcc: 						//CZ adr
			if (state->cc.z == 1)
			{
                uint16_t	ret = state->pc+2;
                WriteMem(state, state->sp-1, (ret >> 8) & 0xff);
                WriteMem(state, state->sp-2, (ret & 0xff));
                state->sp = state->sp - 2;
                state->pc = (opcode[2] << 8) | opcode[1];
			}
			else
				state->pc += 2;
			break;
		case 0xcd: 						//CALL address
        {
			uint16_t	ret = state->pc+2;
            WriteMem(state, state->sp-1, (ret >> 8) & 0xff);
            WriteMem(state, state->sp-2, (ret & 0xff));
			state->sp = state->sp - 2;
			state->pc = (opcode[2] << 8) | opcode[1];
        }
 			break;
		case 0xce:					//ACI  byte 
        {
			uint16_t x = state->a + opcode[1] + state->cc.cy;
            FlagsZSP(state, x&0xff);
			state->cc.cy = (x > 0xff);
			state->a = x & 0xff;
			state->pc++;
        }
			break;
		case 0xcf: 					//RST 1
        {
			uint16_t	ret = state->pc+2;
            WriteMem(state, state->sp-1, (ret >> 8) & 0xff);
            WriteMem(state, state->sp-2, (ret & 0xff));
			state->sp = state->sp - 2;
			state->pc = 0x0008;
        }
			break;
            
		case 0xd0:					//RNC
			if (state->cc.cy == 0)
			{
				state->pc = state->memory[state->sp] | (state->memory[state->sp+1]<<8);
				state->sp += 2;
			}
			break;
		case 0xd1: 						//POP    D
            Pop(state, &state->d, &state->e);
			break;
		case 0xd2:						//JNC
			if (state->cc.cy == 0)
				state->pc = (opcode[2] << 8) | opcode[1];
			else
				state->pc += 2;
			break;
		case 0xd3:                      //OUT d8
			state->pc++;
			break;
		case 0xd4:						//CNC adr
			if (state->cc.cy == 0)
			{
				uint16_t	ret = state->pc+2;
                WriteMem(state, state->sp-1, (ret >> 8) & 0xff);
                WriteMem(state, state->sp-2, (ret & 0xff));
				state->sp = state->sp - 2;
				state->pc = (opcode[2] << 8) | opcode[1];
			}
			else
				state->pc += 2;
			break;
		case 0xd5: 						//PUSH   D
            Push(state, state->d, state->e);
			break;
		case 0xd6: 						//SUI    byte
        {
			uint8_t x = state->a - opcode[1];
            FlagsZSP(state, x&0xff);
			state->cc.cy = (state->a < opcode[1]);
			state->a = x;
			state->pc++;
        }
			break;
		case 0xd7: 					//RST 2
        {
			uint16_t	ret = state->pc+2;
            WriteMem(state, state->sp-1, (ret >> 8) & 0xff);
            WriteMem(state, state->sp-2, (ret & 0xff));
			state->sp = state->sp - 2;
			state->pc = 0x10;
        }
			break;
		case 0xd8: 					//RC
			if (state->cc.cy != 0)
			{
                state->pc = state->memory[state->sp] | (state->memory[state->sp+1]<<8);
                state->sp += 2;
			}
			break;
		case 0xd9: UnimplementedInstruction(state); break;
		case 0xda: 					//JC
			if (state->cc.cy != 0)
				state->pc = (opcode[2] << 8) | opcode[1];
			else
				state->pc += 2;
			break;		
		case 0xdb: 					//IN d8
			state->pc++;
			break;
		case 0xdc: 					//CC adr
			if (state->cc.cy != 0)
			{
				uint16_t	ret = state->pc+2;
                WriteMem(state, state->sp-1, (ret >> 8) & 0xff);
                WriteMem(state, state->sp-2, (ret & 0xff));
				state->sp = state->sp - 2;
				state->pc = (opcode[2] << 8) | opcode[1];
			}
			else
				state->pc += 2;
			break;
		case 0xdd: UnimplementedInstruction(state); break;
		case 0xde: 					//SBI byte
        {
			uint16_t x = state->a - opcode[1] - state->cc.cy;
            FlagsZSP(state, x&0xff);
			state->cc.cy = (x > 0xff);
			state->a = x & 0xff;
			state->pc++;
        }
			break;
		case 0xdf: 					//RST 3
        {
			uint16_t	ret = state->pc+2;
            WriteMem(state, state->sp-1, (ret >> 8) & 0xff);
            WriteMem(state, state->sp-2, (ret & 0xff));
			state->sp = state->sp - 2;
			state->pc = 0x18;
        }
			break;
            
		case 0xe0:					//RPO
			if (state->cc.p == 0)
			{
				state->pc = state->memory[state->sp] | (state->memory[state->sp+1]<<8);
				state->sp += 2;
			}
			break;
		case 0xe1: 					//POP    H
            Pop(state, &state->h, &state->l);
			break;
		case 0xe2: 						//JPO
			if (state->cc.p == 0)
				state->pc = (opcode[2] << 8) | opcode[1];
			else
				state->pc += 2;
			break;
		case 0xe3:						//XTHL 
        {
            uint8_t h = state->h;
            uint8_t l = state->l;
            state->l = state->memory[state->sp];
            state->h = state->memory[state->sp+1]; 
            WriteMem(state, state->sp, l );
            WriteMem(state, state->sp+1, h );
        }
			break;
		case 0xe4: 						//CPO adr
			if (state->cc.p == 0)
			{
				uint16_t	ret = state->pc+2;
                WriteMem(state, state->sp-1, (ret >> 8) & 0xff);
                WriteMem(state, state->sp-2, (ret & 0xff));
				state->sp = state->sp - 2;
				state->pc = (opcode[2] << 8) | opcode[1];
			}
			else
				state->pc += 2;
			break;
		case 0xe5: 						//PUSH   H
            Push(state, state->h, state->l);
			break;
		case 0xe6: 						//ANI    byte
        {
			state->a = state->a & opcode[1];
			LogicFlagsA(state);
			state->pc++;
        }
			break;
		case 0xe7: 					//RST 4
        {
			uint16_t	ret = state->pc+2;
            WriteMem(state, state->sp-1, (ret >> 8) & 0xff);
            WriteMem(state, state->sp-2, (ret & 0xff));
			state->sp = state->sp - 2;
			state->pc = 0x20;
        }
			break;
		case 0xe8: 					//RPE
			if (state->cc.p != 0)
			{
				state->pc = state->memory[state->sp] | (state->memory[state->sp+1]<<8);
				state->sp += 2;
			}
			break;
		case 0xe9: 						//PCHL
			state->pc = (state->h << 8) | state->l;
			break;
		case 0xea: 						//JPE
			if (state->cc.p != 0)
				state->pc = (opcode[2] << 8) | opcode[1];
			else
				state->pc += 2;
			break;		
		case 0xeb: 					//XCHG
        {
            uint8_t save1 = state->d;
            uint8_t save2 = state->e;
            state->d = state->h;
            state->e = state->l;
            state->h = save1;
            state->l = save2;
        }
			break;
		case 0xec: 					//CPE adr
			if (state->cc.p != 0)
			{
				uint16_t	ret = state->pc+2;
                WriteMem(state, state->sp-1, (ret >> 8) & 0xff);
                WriteMem(state, state->sp-2, (ret & 0xff));
				state->sp = state->sp - 2;
				state->pc = (opcode[2] << 8) | opcode[1];
			}
			else
				state->pc += 2;
			break;
		case 0xed: UnimplementedInstruction(state); break;
		case 0xee: 					//XRI data
        {
			uint8_t x = state->a ^ opcode[1];
            FlagsZSP(state, x);
			state->cc.cy = 0;		//data book says clear cy
			state->a = x;
			state->pc++;
        }
			break;
		case 0xef: 					//RST 5
        {
			uint16_t	ret = state->pc+2;
            WriteMem(state, state->sp-1, (ret >> 8) & 0xff);
            WriteMem(state, state->sp-2, (ret & 0xff));
			state->sp = state->sp - 2;
			state->pc = 0x28;
        }
			break;
            
		case 0xf0: 					//RP
			if (state->cc.s == 0)
			{
				state->pc = state->memory[state->sp] | (state->memory[state->sp+1]<<8);
				state->sp += 2;
			}
			break;		
		case 0xf1: 					//POP PSW
            Pop(state, &state->a,(unsigned char*) &state->cc);
			break;
		case 0xf2: 
			if (state->cc.s == 0)
				state->pc = (opcode[2] << 8) | opcode[1];
			else
				state->pc += 2;
			break;
		case 0xf3: state->int_enable = 0;  break;
		case 0xf4: 						//CP
			if (state->cc.s == 0)
			{
				uint16_t	ret = state->pc+2;
                WriteMem(state, state->sp-1, (ret >> 8) & 0xff);
                WriteMem(state, state->sp-2, (ret & 0xff));
				state->sp = state->sp - 2;
				state->pc = (opcode[2] << 8) | opcode[1];
			}
			else
				state->pc += 2;
			break;
            
		case 0xf5: 						//PUSH   PSW
            Push(state, state->a, *(unsigned char*)&state->cc);
			break;
            
		case 0xf6: 						//ORI    byte
        {
			//AC set if lower nibble of h was zero prior to dec
			uint8_t x = state->a | opcode[1];
            FlagsZSP(state, x);
			state->cc.cy = 0;
			state->a = x;
			state->pc++;
        }
			break;
		case 0xf7: 					//RST 6
        {
			uint16_t	ret = state->pc+2;
            WriteMem(state, state->sp-1, (ret >> 8) & 0xff);
            WriteMem(state, state->sp-2, (ret & 0xff));
			state->sp = state->sp - 2;
			state->pc = 0x30;
        }
			break;
		case 0xf8:					//RM
			if (state->cc.s != 0)
			{
				state->pc = state->memory[state->sp] | (state->memory[state->sp+1]<<8);
				state->sp += 2;
			}
			break;
		case 0xf9: 					//SPHL
			state->sp = state->l | (state->h << 8);
			break;
		case 0xfa: 					//JM
			if (state->cc.s != 0)
				state->pc = (opcode[2] << 8) | opcode[1];
			else
				state->pc += 2;
			break;
		case 0xfb: state->int_enable = 1;  break;
		case 0xfc: 					//CM
			if (state->cc.s != 0)
			{
				uint16_t	ret = state->pc+2;
                WriteMem(state, state->sp-1, (ret >> 8) & 0xff);
                WriteMem(state, state->sp-2, (ret & 0xff));
				state->sp = state->sp - 2;
				state->pc = (opcode[2] << 8) | opcode[1];
			}
			else
				state->pc += 2;
			break;
            
		case 0xfd: UnimplementedInstruction(state); break;
		case 0xfe: 						//CPI  byte
        {
			uint8_t x = state->a - opcode[1];
            FlagsZSP(state, x);
			state->cc.cy = (state->a < opcode[1]);
			state->pc++;
        }
			break;
		case 0xff: 					//RST 7
        {
			uint16_t	ret = state->pc+2;
            WriteMem(state, state->sp-1, (ret >> 8) & 0xff);
            WriteMem(state, state->sp-2, (ret & 0xff));
			state->sp = state->sp - 2;
			state->pc = 0x38;
        }
			break;
	}

#if PRINTOPS
	printf("\t");
	printf("%c", state->cc.z ? 'z' : '.');
	printf("%c", state->cc.s ? 's' : '.');
	printf("%c", state->cc.p ? 'p' : '.');
	printf("%c", state->cc.cy ? 'c' : '.');
	printf("%c  ", state->cc.ac ? 'a' : '.');
	printf("A $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x SP %04x\n", state->a, state->b, state->c,
           state->d, state->e, state->h, state->l, state->sp);
#endif
	return cycles8080[*opcode];
}

void GenerateInterrupt(State8080* state, int interrupt_num)
{
	//perform "PUSH PC"
    Push(state, (state->pc & 0xFF00) >> 8, (state->pc & 0xff));
	
	//Set the PC to the low memory vector
	state->pc = 8 * interrupt_num;
    
    //"DI"
    state->int_enable = 0;
}



