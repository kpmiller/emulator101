typedef struct Chip8State {
	uint8_t		V[16];
	uint16_t	I;
	uint16_t	SP;
	uint16_t	PC;
	uint8_t		delay;
	uint8_t		sound;
	uint8_t		*memory;
	uint8_t		*screen;  //this is memory[0xF00];
	uint8_t		key_state[16];
	uint8_t		save_key_state[16];
	int			waiting_for_key;
    uint8_t     halt;
} Chip8State;




Chip8State* InitChip8(void);
void EmulateChip8Op(Chip8State *state);
