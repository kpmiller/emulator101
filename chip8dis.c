#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

static void DisassembleChip8Op(unsigned char *codebuffer, int pc)
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
				uint8_t lastnib = code[1]>>4;
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
}

int main (int argc, char**argv)
{
	FILE *f= fopen(argv[1], "rb");
	if (f==NULL)
	{
		printf("error: Couldn't open %s\n", argv[1]);
		exit(1);
	}
	
	//Get the file size
	fseek(f, 0L, SEEK_END);
	int fsize = ftell(f);
	fseek(f, 0L, SEEK_SET);

	//CHIP-8 convention puts programs in memory at 0x200
	// They will all have hardcoded addresses expecting that
	//
	//Read the file into memory at 0x200 and close it.
	unsigned char *buffer=malloc(fsize+0x200);
	fread(buffer+0x200, fsize, 1, f);
	fclose(f);

	int pc = 0x200;
	while (pc < (fsize+0x200))
	{
		DisassembleChip8Op(buffer, pc);
		pc += 2;
		printf ("\n");
	}
	return 0;
}
