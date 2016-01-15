//
//  Chip8View.m
//

#import "Chip8View.h"

@implementation Chip8View

- (id)initWithFrame:(NSRect)frame {
	
	NSOpenGLPixelFormatAttribute att[] = 
	{
		NSOpenGLPFAWindow,
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAColorSize, 24,
		NSOpenGLPFAAccelerated,
		0
	};
	
	NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:att]; 
	
	if(self = [super initWithFrame:frame pixelFormat:pixelFormat]) {
        buffer8 = calloc( 64*32,1);
        
        //a 16ms time interval to get 60 fps
        renderTimer = [NSTimer timerWithTimeInterval:0.016
                                              target:self
                                            selector:@selector(timerFired:)
                                            userInfo:nil
                                             repeats:YES];
        
        [[NSRunLoop currentRunLoop] addTimer:renderTimer forMode:NSDefaultRunLoopMode];
   }
	
	[pixelFormat release];

    chip8 = [[Chip8Machine alloc] init];
    [chip8 startEmulation];

	return self;
}

- (void)drawRect:(NSRect)rect {
    [self.openGLContext makeCurrentContext];
    int i;
    
    //Convert the game's 1-bit bitmap into 
    // something OpenGL can swallow - in this case
    // GL_LUMINANCE
    uint8_t *b8 = buffer8;
    uint8_t *fb = [chip8 framebuffer];
    
    for (i=0; i < ((64/8)*32); i++)
    {
        uint8_t bw_pix = fb[0];
        if (bw_pix & 0x80) b8[0] = 0xFF; else b8[0] = 0;
        if (bw_pix & 0x40) b8[1] = 0xFF; else b8[1] = 0;
        if (bw_pix & 0x20) b8[2] = 0xFF; else b8[2] = 0;
        if (bw_pix & 0x10) b8[3] = 0xFF; else b8[3] = 0;
        if (bw_pix & 0x08) b8[4] = 0xFF; else b8[4] = 0;
        if (bw_pix & 0x04) b8[5] = 0xFF; else b8[5] = 0;
        if (bw_pix & 0x02) b8[6] = 0xFF; else b8[6] = 0;
        if (bw_pix & 0x01) b8[7] = 0xFF; else b8[7] = 0;           
        b8 += 8;
        fb++;
    }
    
	//Create the texture.  Have to set the FILTER so GL knows the
	// texture is not mipmapped
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 64,32, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer8);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
	glClearColor(0,0,0,0);  
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //Draw the textured quad
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0);
    glVertex2f(-1.0, -1.0);
    glTexCoord2f(1.0, 1.0);
    glVertex2f(1.0, -1.0);
    glTexCoord2f(1.0, 0.0);
    glVertex2f(1.0, 1.0);
    glTexCoord2f(0.0, 0.0);
    glVertex2f(-1.0, 1.0);
    glEnd();
    
    [self.openGLContext flushBuffer];
}

// Timer callback method
- (void)timerFired:(id)sender
{
    [self setNeedsDisplay:YES];
}

-(void) reshape
{
    [self.openGLContext makeCurrentContext];
    
    //I want to support Lion fullscreen mode without making it all stretchy
    // So I'm going to use glViewport to put the game in the biggest
    // even multiple of 224x256 that will fit in the current window.
    
    float x, y, w, h, scale;
    
    x = self.bounds.origin.x;
    y = self.bounds.origin.y;
    w = self.bounds.size.width;
    h = self.bounds.size.height;
    
    if (w>h)
        //Figure out what multiple of h will fit in the window,
        scale = floorf(h/32.0);
    else
        scale = floorf(w/64.0);
    
    w = 64.0 * scale;
    h = 32.0 * scale;
    
    x = x + ((self.bounds.size.width/2.0) - (w/2.0));
    y = y + ((self.bounds.size.height/2.0) - (h/2.0));
    
    glViewport(x, y, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    
    //Try to contstrain resizing to multiples of the game size
    [[self window] setContentResizeIncrements:NSMakeSize(64, 32)];
}

//************************************
#pragma mark Key Handling
//************************************

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void) keyDown: (NSEvent *) event
{
    NSString *characters;
    characters = [event characters];
    
    unichar character;
    character = [characters characterAtIndex: 0];
    
    //Map the chip-8 standard direction keys to the arrows 
    switch (character) {
        case NSLeftArrowFunctionKey:
            character = '4';
            break;
            
        case NSRightArrowFunctionKey:
            character = '6';
            break;
            
        case NSUpArrowFunctionKey:
            character = '8';
            break;
            
        case NSDownArrowFunctionKey:
            character = '2';
            break;
            
        default:
            break;
    }
    [chip8 KeyDown:character];
}

- (void) keyUp: (NSEvent *) event
{
    NSString *characters;
    characters = [event characters];
    
    unichar character;
    character = [characters characterAtIndex: 0];
    
    //Map the chip-8 standard direction keys to the arrows 
    switch (character) {
        case NSLeftArrowFunctionKey:
            character = '4';
            break;
            
        case NSRightArrowFunctionKey:
            character = '6';
            break;
            
        case NSUpArrowFunctionKey:
            character = '8';
            break;
            
        case NSDownArrowFunctionKey:
            character = '2';
            break;
            
        default:
            break;
    }
    [chip8 KeyUp:character];
}



@end
