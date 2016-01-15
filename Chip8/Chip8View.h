//
//  Chip8View.h
//

#import <AppKit/AppKit.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import "Chip8Machine.h"

@interface Chip8View : NSOpenGLView
{
    NSTimer                *renderTimer;
    Chip8Machine           *chip8;
    unsigned char          *buffer8;

}

- (void)timerFired:(id)sender;

@end
