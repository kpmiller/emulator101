//
//  SpaceInvadersMachine.h
//  PhoneInvaders
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#include "8080emu.h"
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

#define BUTTON_COIN 'c'
#define BUTTON_P1_LEFT 'a'
#define BUTTON_P1_RIGHT 's'
#define BUTTON_P1_FIRE ' '
#define BUTTON_P1_START '1'

@interface SpaceInvadersMachine : NSObject <AVAudioPlayerDelegate>
{
    State8080   *state;
    
    double      lastTimer;
    double      nextInterrupt;
    int         whichInterrupt;
    
    NSTimer     *emulatorTimer;
    
    uint8_t     shift0;         //LSB of Space Invader's external shift hardware
    uint8_t     shift1;         //MSB
    uint8_t     shift_offset;         // offset for external shift hardware
    
    uint8_t     in_port1;
    uint8_t     out_port3, out_port5, last_out_port3, last_out_port5;
    
    AVAudioPlayer *ufo;
}

@property NSMutableArray *soundeffects;

-(double) timeusec;

-(void) ReadFile:(NSString*)filename IntoMemoryAt:(uint32_t)memoffset;
-(id) init;

-(void) doCPU;
-(void) startEmulation;

-(void *) framebuffer;

- (void) ButtonDown: (uint8_t) key;
- (void) ButtonUp: (uint8_t) key;

@end
