//
//  InvadersView.m
//  Invaders
//
//  Created by Emulator101 on 11/11/11.
//
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

#import "InvadersView.h"

#define RGB_ON   0xFFFFFFFFL
#define RGB_OFF 0x00000000L

@implementation InvadersView

- (void)awakeFromNib
{
    invaders = [[SpaceInvadersMachine alloc] init];
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    buffer8888 = malloc(4 * 224*256);
    bitmapCtx = CGBitmapContextCreate(buffer8888, 224, 256, 8, 224*4, colorSpace, kCGImageAlphaNoneSkipFirst);
    
    //a 16ms time interval to get 60 fps
    renderTimer = [NSTimer timerWithTimeInterval:0.016
                                          target:self
                                        selector:@selector(timerFired:)
                                        userInfo:nil
                                         repeats:YES];
    
    [[NSRunLoop currentRunLoop] addTimer:renderTimer forMode:NSDefaultRunLoopMode];
    [invaders startEmulation];
}


- (void)drawRect:(NSRect)dirtyRect
{
    int i, j;
    
    
    //Translate the 1-bit space invaders frame buffer into
    // my 32bpp RGB bitmap.  We have to rotate and
    // flip the image as we go.
    //
    unsigned char *b = (unsigned char *)buffer8888;
    unsigned char *fb = [invaders framebuffer];
    for (i=0; i< 224; i++)
    {
        for (j = 0; j < 256; j+= 8)
        {
            int p;
            //Read the first 1-bit pixel
            // divide by 8 because there are 8 pixels
            // in a byte
            unsigned char pix = fb[(i*(256/8)) + j/8];
            
            //That makes 8 output vertical pixels
            // we need to do a vertical flip
            // so j needs to start at the last line
            // and advance backward through the buffer
            int offset = (255-j)*(224*4) + (i*4);
            unsigned int*p1 = (unsigned int*)(&b[offset]);
            for (p=0; p<8; p++)
            {
                if ( 0!= (pix & (1<<p)))
                    *p1 = RGB_ON;
                else
                    *p1 = RGB_OFF;
                p1-=224;  //next line
            }
        }
    }
    
    
    CGContextRef myContext = [[NSGraphicsContext currentContext] graphicsPort];
    CGImageRef ir = CGBitmapContextCreateImage(bitmapCtx);
    CGContextDrawImage(myContext, self.bounds, ir);
    CGImageRelease(ir);
}

// Timer callback method
- (void)timerFired:(id)sender
{
    [self setNeedsDisplay:YES];
}


@end
