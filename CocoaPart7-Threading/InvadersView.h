//
//  InvadersView.h
//  Invaders
//

#import <Cocoa/Cocoa.h>
#import "SpaceInvadersMachine.h"

@interface InvadersView : NSView
{
    NSTimer         *renderTimer;
    SpaceInvadersMachine   *invaders;
    
    CGContextRef        bitmapCtx;
    unsigned char       *buffer8888;
}

- (void)timerFired:(id)sender;


@end
