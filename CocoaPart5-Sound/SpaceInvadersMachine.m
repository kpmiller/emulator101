//
//  SpaceInvadersMachine.m
//  Invaders
//

#import "SpaceInvadersMachine.h"

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

#define USE_THREADS 1

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
        NSLog(@"corrupted file %@?  shouldn't be this big: %ld bytes", filename, [data length]);
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
    
    in_port1 = 0;
    
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
    unsigned char a = 0;
    switch(port)
    {
        case 0:
            return 0xf;
            break;
        case 1:
            return in_port1;
            break;
        case 2:
            return 0;
            break;
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
        case 3:
            out_port3 = value;
            break;
        case 4:
            shift0 = shift1;
            shift1 = value;
            break;
        case 5:
            out_port5 = value;
            break;
    }
    
}

-(void) PlaySounds
{
    if (out_port3 != last_out_port3)
    {
        if ( (out_port3 & 0x1) && !(last_out_port3 & 0x1))
        {
            //start UFO
            ufo = [NSSound soundNamed:@"0.wav"];
            [ufo setLoops:YES];
            [ufo play];
        }
        else if ( !(out_port3 & 0x1) && (last_out_port3 & 0x1))
        {
            //stop UFO
            if (ufo)
            {
                [ufo stop];
                ufo = NULL;
            }
        }
        
        if ( (out_port3 & 0x2) && !(last_out_port3 & 0x2))
            [[NSSound soundNamed:@"1.wav"] play];
        
        if ( (out_port3 & 0x4) && !(last_out_port3 & 0x4))
            [[NSSound soundNamed:@"2.wav"] play];
        
        if ( (out_port3 & 0x8) && !(last_out_port3 & 0x8))
            [[NSSound soundNamed:@"3.wav"] play];
        
        last_out_port3 = out_port3;
    }
    if (out_port5 != last_out_port5)
    {
        if ( (out_port5 & 0x1) && !(last_out_port5 & 0x1))
            [[NSSound soundNamed:@"4.wav"] play];
        
        if ( (out_port5 & 0x2) && !(last_out_port5 & 0x2))
            [[NSSound soundNamed:@"5.wav"] play];
        
        if ( (out_port5 & 0x4) && !(last_out_port5 & 0x4))
            [[NSSound soundNamed:@"6.wav"] play];
        
        if ( (out_port5 & 0x8) && !(last_out_port5 & 0x8))
            [[NSSound soundNamed:@"7.wav"] play];
        
        if ( (out_port5 & 0x10) && !(last_out_port5 & 0x10))
            [[NSSound soundNamed:@"8.wav"] play];
        
        last_out_port5 = out_port5;
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
            [self PlaySounds];
        }
        else
            cycles += Emulate8080Op(state);
        
    }
    lastTimer  = now;
}

- (void) startEmulation
{
    lastTimer = [self timeusec];
    emulatorTimer = [NSTimer scheduledTimerWithTimeInterval: 0.001
                                                     target: self
                                                   selector:@selector(doCPU)
                                                   userInfo: nil repeats:YES];
}

- (void) KeyDown: (uint8_t) key
{
    switch (key)
    {
        case KEY_COIN:
            in_port1 |= 0x1;
            break;
        case KEY_P1_LEFT:
            in_port1 |= 0x20;
            break;
        case KEY_P1_RIGHT:
            in_port1 |= 0x40;
            break;
        case KEY_P1_FIRE:
            in_port1 |= 0x10;
            break;
        case KEY_P1_START:
            in_port1 |= 0x04;
            break;
        case KEY_PAUSE:
            if (paused)
            {
                [self startEmulation];
                paused = NO;
            }
            else
            {
                [emulatorTimer invalidate];
                emulatorTimer = nil;
                paused = YES;
            }
            break;
    }
}


- (void) KeyUp: (uint8_t) key
{
    switch (key)
    {
        case KEY_COIN:
            in_port1 &= ~0x1;
            break;
        case KEY_P1_LEFT:
            in_port1 &= ~0x20;
            break;
        case KEY_P1_RIGHT:
            in_port1 &= ~0x40;
            break;
        case KEY_P1_FIRE:
            in_port1 &= ~0x10;
            break;
        case KEY_P1_START:
            in_port1 &= ~0x04;
            break;
    }
}


@end
