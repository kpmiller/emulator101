//
//  ViewController.m
//  PhoneInvaders
//

#import "ViewController.h"
#import "SpaceInvadersMachine.h"
#import <OpenGLES/ES1/gl.h>

@interface ViewController () {
    uint8_t     *buffer8;
    SpaceInvadersMachine *invaders;
    int last_touchstate[5];
}
@property (strong, nonatomic) EAGLContext *context;

- (void)setupGL;
- (void)tearDownGL;

@end

@implementation ViewController

@synthesize context = _context;

- (void)dealloc
{
    [_context release];
    [super dealloc];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.context = [[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1] autorelease];

    if (!self.context) {
        NSLog(@"Failed to create ES context");
    }
    
    GLKView *view = (GLKView *)self.view;
    view.context = self.context;
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    
    [self setupGL];
}

- (void)viewDidUnload
{    
    [super viewDidUnload];
    
    [self tearDownGL];
    
    if ([EAGLContext currentContext] == self.context) {
        [EAGLContext setCurrentContext:nil];
    }
    self.context = nil;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Release any cached data, images, etc. that aren't in use.
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
        return (interfaceOrientation != UIInterfaceOrientationPortraitUpsideDown);
    } else {
        return YES;
    }
}

- (void)setupGL
{
    [EAGLContext setCurrentContext:self.context];
    
    buffer8 = calloc( 256*256, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 256, 256, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer8);
    
    invaders = [[SpaceInvadersMachine alloc] init];
    [invaders startEmulation];
}

- (void)tearDownGL
{
    [EAGLContext setCurrentContext:self.context];
}

#pragma mark - GLKView and GLKViewController delegate methods

- (void)update
{
    int i;
    uint8_t *b8 = buffer8 + (224*256)-8;
    uint8_t *fb = [invaders framebuffer];
    
    for (i=0; i < ((256/8)*224); i++)
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
        b8 -= 8;
        fb++;
    }

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 224, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer8);    
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
    float gameAspect = (256.0/224.0);
    float heightRatio = (rect.size.width * gameAspect)/rect.size.height;
    float y = (2.0*heightRatio);
    
    const GLfloat vtx[] = { 
        -1.0, 1.0-y,
        1.0, 1.0-y,
        -1.0, 1.0,
        1.0, 1.0,
    }; 
    
    const GLfloat tc[] = {
        1.0, 0.875,
        1.0, 0.0,
        0.0, 0.875,
        0.0, 0.0,
    };
    
    
    glClear(GL_COLOR_BUFFER_BIT); 
    
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glVertexPointer(2, GL_FLOAT, 0, vtx); 
    glTexCoordPointer(2, GL_FLOAT, 0, tc);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_TEXTURE_2D);
            
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); 
        
}

-(void) checkTouches:(NSSet *)touches ending:(BOOL) ending
{
    struct TouchZones {
        CGRect bounds;
        uint8_t message;
        uint8_t keystate_index;
    };
    
    struct TouchZones tz[] = 
    {
        { CGRectMake(0.0, 0.0, 150.0, 150.0), BUTTON_COIN, 0 },
        { CGRectMake(170.0, 0.0, 150.0, 150.0), BUTTON_P1_START, 1 },
        { CGRectMake(0.0, 330.0, 80.0, 150.0), BUTTON_P1_LEFT, 2 },
        { CGRectMake(80.0, 330.0, 80.0, 150.0), BUTTON_P1_RIGHT, 3 },
        { CGRectMake(170.0, 330.0, 160.0, 150.0), BUTTON_P1_FIRE, 4 },
        
        { CGRectMake(0.0, 0.0, 150.0, 150.0), 0, 0}
    };
    
    uint8_t newstates[5] = {0,0,0,0,0};
    
    if ([touches count] > 0) 
    {
        for (UITouch *touch in touches) 
        {
            CGPoint	p = [touch locationInView:self.view];
            struct TouchZones *z = tz;
            
            while (z->message != 0)
            {
                if ((CGRectContainsPoint(z->bounds, p)) && ! ending)
                    newstates[z->keystate_index] = 1; 
                if ((CGRectContainsPoint(z->bounds, p)) && ending)
                    newstates[z->keystate_index] = 0;                 
                
                z++;
            }
        }
    }
    
    //Now we have an array of all the touches down.
    //Compare them against the last touches:
    // if the state has changed from not-touched to touched, send a key down
    // if the state has changes from touched to not-touched, send a key up
    struct TouchZones *z = tz;
    
    while (z->message != 0)
    {
        int i = z->keystate_index;
        if ((newstates[i] == 0) && (last_touchstate[i] == 1))
            [invaders ButtonUp:z->message];
        else if ((newstates[i] == 1) && (last_touchstate[i] == 0))
            [invaders ButtonDown:z->message];
        last_touchstate[i] = newstates[i];
        z++;
    }
    
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *) event {
	[self checkTouches:[event allTouches] ending:NO];
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
	[self checkTouches:[event allTouches] ending:YES];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
	[self checkTouches:[event allTouches] ending:YES];
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
	[self checkTouches:[event allTouches] ending:NO];
}


@end
