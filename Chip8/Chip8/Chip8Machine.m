//
//  Chip8Machine.m
//  Chip8
//

#import "Chip8Machine.h"
#include <sys/time.h>

@implementation Chip8Machine


-(void) ReadFile:(NSString*)filename IntoMemoryAt:(uint32_t)memoffset
{
    
    NSBundle *mainbundle = [NSBundle mainBundle];
    NSString *filepath = [mainbundle pathForResource:filename ofType:NULL];
    
    NSData *data = [[NSData alloc]initWithContentsOfFile:filepath];
    
    if (data == NULL)
    {
        NSLog(@"Open of %@ failed.  This is not going to go well.", filename);
        return;
    }
    if ([data length] > 0x800)
    {
        NSLog(@"corrupted file %@?  shouldn't be this big: %ld bytes", filename, [data length]);
    }
    
	uint8_t *buffer = &state->memory[memoffset];
    memcpy(buffer, [data bytes], [data length]);
}

-(id) init
{
    state = InitChip8();
    
	[self ReadFile:@"Maze.ch8" IntoMemoryAt:0x200];
    
    lastTick = 0.0;
    
    return self;
}


-(void *) framebuffer
{
    return (void*) &state->memory[0xF00];
}

-(double) timeusec
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return ((double)time.tv_sec * 1E6) + ((double)time.tv_usec);
}

-(void) handleTimers:(double)now
{
    //Decrement the timers
    if (now - lastTick > 16667.0) //1/60Hz, or 16.667ms
    {
        uint32_t tickspast = (int) (now - lastTick);
        tickspast /= 16667;
        
        if (state->sound > 0)
            state->sound = state->sound - ((state->sound > tickspast) ? tickspast : state->sound);
        if (state->delay > 0)
            state->delay = state->delay - ((state->delay > tickspast) ? tickspast : state->delay);
        lastTick = now;
    }
}

-(void) doCPU
{    
    double now = [self timeusec];
    
    if (lastTick == 0.0)
        lastTick = now;
    
    if (state->halt) return;
    
    [self handleTimers:now];
    int cycles_to_catch_up = (1800.0 * ((now - lastTimer)/1e6));
    int cycles = 0;
    
    while (cycles_to_catch_up > cycles)
    {
        cycles += 1;
        if (!state->halt) 
            EmulateChip8Op(state);
    }
    lastTimer  = now;
}

- (void) startEmulation
{
    emulatorTimer = [NSTimer scheduledTimerWithTimeInterval: 0.1
                                                     target: self
                                                   selector:@selector(doCPU)
                                                   userInfo: nil repeats:YES];
}

- (void) KeyDown: (uint8_t) key
{
    switch (key)
    {
        case '0': state->key_state[0] = 1; break;
        case '1': state->key_state[1] = 1; break;
        case '2': state->key_state[2] = 1; break;
        case '3': state->key_state[3] = 1; break;
        case '4': state->key_state[4] = 1; break;
        case '5': state->key_state[5] = 1; break;
        case '6': state->key_state[6] = 1; break;
        case '7': state->key_state[7] = 1; break;
        case '8': state->key_state[8] = 1; break;
        case '9': state->key_state[9] = 1; break;
        case 'a': state->key_state[0xa] = 1; break;
        case 'A': state->key_state[0xa] = 1; break;
        case 'b': state->key_state[0xb] = 1; break;
        case 'B': state->key_state[0xb] = 1; break;
        case 'c': state->key_state[0xc] = 1; break;
        case 'C': state->key_state[0xc] = 1; break;
        case 'd': state->key_state[0xd] = 1; break;
        case 'D': state->key_state[0xd] = 1; break;
        case 'e': state->key_state[0xe] = 1; break;
        case 'E': state->key_state[0xe] = 1; break;
        case 'f': state->key_state[0xf] = 1; break;
        case 'F': state->key_state[0xf] = 1; break;
        default:
            return;
    }
}


- (void) KeyUp: (uint8_t) key
{
    switch (key)
    {
        case '0': state->key_state[0] = 0; break;
        case '1': state->key_state[1] = 0; break;
        case '2': state->key_state[2] = 0; break;
        case '3': state->key_state[3] = 0; break;
        case '4': state->key_state[4] = 0; break;
        case '5': state->key_state[5] = 0; break;
        case '6': state->key_state[6] = 0; break;
        case '7': state->key_state[7] = 0; break;
        case '8': state->key_state[8] = 0; break;
        case '9': state->key_state[9] = 0; break;
        case 'a': state->key_state[0xa] = 0; break;
        case 'A': state->key_state[0xa] = 0; break;
        case 'b': state->key_state[0xb] = 0; break;
        case 'B': state->key_state[0xb] = 0; break;
        case 'c': state->key_state[0xc] = 0; break;
        case 'C': state->key_state[0xc] = 0; break;
        case 'd': state->key_state[0xd] = 0; break;
        case 'D': state->key_state[0xd] = 0; break;
        case 'e': state->key_state[0xe] = 0; break;
        case 'E': state->key_state[0xe] = 0; break;
        case 'f': state->key_state[0xf] = 0; break;
        case 'F': state->key_state[0xf] = 0; break;
    }
}

@end
