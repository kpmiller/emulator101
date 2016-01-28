#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

//Some code cares that these flags are in exact 
// right bits when.  For instance, some code
// "pops" values into the PSW that they didn't push.
//
typedef struct {
	uint8_t		cy:1;
	uint8_t		pad:1;
	uint8_t		p:1;
	uint8_t		pad2:1;
	uint8_t		ac:1;
	uint8_t		pad3:1;
	uint8_t		z:1;
	uint8_t		s:1;
} ConditionCodes;

typedef struct  {
	uint8_t		a;
	uint8_t		b;
	uint8_t		c;
	uint8_t		d;
	uint8_t		e;
	uint8_t		h;
	uint8_t		l;
	uint16_t	sp;
	uint16_t	pc;
	uint8_t		*memory;
	ConditionCodes		cc;
	uint8_t		int_enable;

} State8080;



int Emulate8080Op(State8080* state);
void GenerateInterrupt(State8080* state, int interrupt_num);
