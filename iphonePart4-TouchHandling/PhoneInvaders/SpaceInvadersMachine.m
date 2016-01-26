//
//  SpaceInvadersMachine.m
//  PhoneInvaders
//

#import "SpaceInvadersMachine.h"
#include <sys/time.h>

@implementation SpaceInvadersMachine
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
        NSLog(@"corrupted file %@?  shouldn't be this big: %d bytes", filename, (uint32_t) [data length]);
    }
    
	uint8_t *buffer = &state->memory[memoffset];
    memcpy(buffer, [data bytes], [data length]);
}

-(id) init
{
    state = calloc(sizeof(State8080), 1);
    state->memory = malloc(16 * 0x1000);    
    
	[self ReadFile:@"invaders.h" IntoMemoryAt:0];
	[self ReadFile:@"invaders.g" IntoMemoryAt:0x800];
	[self ReadFile:@"invaders.f" IntoMemoryAt:0x1000];
	[self ReadFile:@"invaders.e" IntoMemoryAt:0x1800];
    
    return self;
}


-(void *) framebuffer
{
    return (void*) &state->memory[0x2400];
}

-(double) timeusec
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return ((double)time.tv_sec * 1E6) + ((double)time.tv_usec);
}

-(uint8_t) InSpaceInvaders:(uint8_t) port
{
    unsigned char a=0;
    switch(port)
    {
        case 1:
            return in_port1;
        case 3:
        {
            uint16_t v = (shift1<<8) | shift0;
            a = ((v >> (8-shift_offset)) & 0xff);
        }
            break;
    }
    return a;
}

-(void) OutSpaceInvaders:(uint8_t) port value:(uint8_t)value
{
    switch(port)
    {
        case 2:
            shift_offset = value & 0x7;
            break;
        case 4:
            shift0 = shift1;
            shift1 = value;
            break;
    }
    
}

-(void) doCPU
{    
    double now = [self timeusec];
    
    if (lastTimer == 0.0)
    {   
        lastTimer = now;
        nextInterrupt = lastTimer + 16000.0;
        whichInterrupt = 1;
    }
    
    if ((state->int_enable) && (now > nextInterrupt))
    {
        if (whichInterrupt == 1)
        {
            GenerateInterrupt(state, 1);
            whichInterrupt = 2;
        }
        else
        {
            GenerateInterrupt(state, 2);
            whichInterrupt = 1;
        }    
        nextInterrupt = now+8000.0;
    }
    
    
    //How much time has passed?  How many instructions will it take to keep up with
    // the current time?  Assume:
    //CPU is 2 MHz
    // so 2M cycles/sec
    
    double sinceLast = now - lastTimer;
    int cycles_to_catch_up = 2 * sinceLast;
    int cycles = 0;
    
    while (cycles_to_catch_up > cycles)
    {
        unsigned char *op;
        op = &state->memory[state->pc];
        if (*op == 0xdb) //machine specific handling for IN
        {
            state->a = [self InSpaceInvaders:op[1]];
            state->pc += 2;
            cycles+=3;
        }
        else if (*op == 0xd3) //machine specific handling for OUT
        {
            [self OutSpaceInvaders:op[1] value:state->a];
            state->pc += 2;
            cycles+=3;
        }
        else
            cycles += Emulate8080Op(state);
        
    }
    lastTimer  = now;
}

- (void) startEmulation
{
    emulatorTimer = [NSTimer scheduledTimerWithTimeInterval: 0.001
                                                     target: self
                                                   selector:@selector(doCPU)
                                                   userInfo: nil repeats:YES];
}

- (void) ButtonDown: (uint8_t) e
{
    switch (e)
    {
        case BUTTON_COIN:
            in_port1 |= 0x1;
            break;
        case BUTTON_P1_LEFT:
            in_port1 |= 0x20;
            break;
        case BUTTON_P1_RIGHT:
            in_port1 |= 0x40;
            break;
        case BUTTON_P1_FIRE:
            in_port1 |= 0x10;
            break;
        case BUTTON_P1_START:
            in_port1 |= 0x04;
            break;
    }
}


- (void) ButtonUp: (uint8_t) e
{
    switch (e)
    {
        case BUTTON_COIN:
            in_port1 &= ~0x1;
            break;
        case BUTTON_P1_LEFT:
            in_port1 &= ~0x20;
            break;
        case BUTTON_P1_RIGHT:
            in_port1 &= ~0x40;
            break;
        case BUTTON_P1_FIRE:
            in_port1 &= ~0x10;
            break;
        case BUTTON_P1_START:
            in_port1 &= ~0x04;
            break;
    }
}

@end
