//
//  GameViewController.m
//  PhoneInvaders
//

#import "GameViewController.h"
#import <OpenGLES/ES2/glext.h>
#import "SpaceInvadersMachine.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

// Attribute index.
enum
{
    ATTRIB_VERTEX,
    ATTRIB_TEX_COORD,
    NUM_ATTRIBUTES
};

@interface GameViewController () {
    GLuint _program;
    
    uint8_t     *buffer8;
    SpaceInvadersMachine *invaders;
    
    GLuint _vertexArray;
    GLuint _vertexBuffer;
}
@property (strong, nonatomic) EAGLContext *context;

- (void)setupGL;
- (void)tearDownGL;

- (BOOL)loadShaders;
- (BOOL)compileShader:(GLuint *)shader type:(GLenum)type file:(NSString *)file;
- (BOOL)linkProgram:(GLuint)prog;
- (BOOL)validateProgram:(GLuint)prog;
@end

@implementation GameViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];

    if (!self.context) {
        NSLog(@"Failed to create ES context");
    }
    
    GLKView *view = (GLKView *)self.view;
    view.context = self.context;
    view.drawableDepthFormat = GLKViewDrawableDepthFormatNone;
    
    [self setupGL];
}

- (void)dealloc
{    
    [self tearDownGL];
    
    if ([EAGLContext currentContext] == self.context) {
        [EAGLContext setCurrentContext:nil];
    }
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];

    if ([self isViewLoaded] && ([[self view] window] == nil)) {
        self.view = nil;
        
        [self tearDownGL];
        
        if ([EAGLContext currentContext] == self.context) {
            [EAGLContext setCurrentContext:nil];
        }
        self.context = nil;
    }

    // Dispose of any resources that can be recreated.
}

- (BOOL)prefersStatusBarHidden {
    return YES;
}

- (void)setupGL
{
    [EAGLContext setCurrentContext:self.context];
    
    glClearColor(0.0, 0.0, 0.0, 1.0);

    [self loadShaders];
    glUseProgram(_program);

    //The viewport of the template is a view stretched from corner to corner
    // of the screen.  I know I'll be in portrait (long side of the phone
    // up.  But since I don't know what phone I'm on, I don't know the ratio
    // of height to width that will maximize the viewable space.  This code
    // figures that out, and adjusts the values of the position to account
    // for that.
    //
    //
    float screenAspect = fabs(self.view.bounds.size.width / self.view.bounds.size.height);
    float invadersAspect = 224.0/256.0;
    float aspect = screenAspect / invadersAspect;
    
    GLfloat gGameVertexData[] =
    {
        // Data layout for each line below is:
        // positionX, positionY,     texture coord s, texture coord t,
        //      v3---v4
        //      | \   |
        //      |  \  |
        //      |   \ |
        //      v1---v2
        -1.0f, -aspect,     0.0f, 0.0f,
         1.0f, -aspect,     0.0f, 1.0f,
        -1.0f,  aspect,     1.0f, 0.0f,
         1.0f,  aspect,     1.0f, 1.0f,
    };
    

    glGenVertexArraysOES(1, &_vertexArray);
    glBindVertexArrayOES(_vertexArray);
    
    glGenBuffers(1, &_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gGameVertexData), gGameVertexData, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, BUFFER_OFFSET(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, BUFFER_OFFSET(8));
    
    //texture setup code
    {
        buffer8 = calloc( 256*224, 1);
        memset(buffer8, 0xCC, (256*224));
        //Define the texture
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 256, 224, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer8);

        //Set the texture to use linear sampling.  The upside is it will
        // be scaled and smoothed.  The downside is that the smoothing will
        // make it look blurry.  Try "GL_NEAREST" instead of "GL_LINEAR" and
        // see if you like it better.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //Since the texture is 224 high (so not a power of two), the wrap
        // mode most be GL_CLAMP_TO_EDGE
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        //Tell the shader which texture to sample
        GLuint tsamplerUniformLocation = glGetUniformLocation(_program, "tsampler");
        glUniform1i(tsamplerUniformLocation, 0);
    }
    
    invaders = [[SpaceInvadersMachine alloc] init];
    [invaders startEmulation];

}

- (void)tearDownGL
{
    [EAGLContext setCurrentContext:self.context];
    
    glDeleteBuffers(1, &_vertexBuffer);
    glDeleteVertexArraysOES(1, &_vertexArray);
    
    if (_program) {
        glDeleteProgram(_program);
        _program = 0;
    }
}

#pragma mark - GLKView and GLKViewController delegate methods

- (void)update
{
    int i;
    uint8_t *b8 = buffer8;
    uint8_t *fb = [invaders framebuffer];
    
    for (i=0; i < ((256/8)*224); i++)
    {
        uint8_t bw_pix = fb[0];
        if (bw_pix & 0x01) b8[0] = 0xFF; else b8[0] = 0;
        if (bw_pix & 0x02) b8[1] = 0xFF; else b8[1] = 0;
        if (bw_pix & 0x04) b8[2] = 0xFF; else b8[2] = 0;
        if (bw_pix & 0x08) b8[3] = 0xFF; else b8[3] = 0;
        if (bw_pix & 0x10) b8[4] = 0xFF; else b8[4] = 0;
        if (bw_pix & 0x20) b8[5] = 0xFF; else b8[5] = 0;
        if (bw_pix & 0x40) b8[6] = 0xFF; else b8[6] = 0;
        if (bw_pix & 0x80) b8[7] = 0xFF; else b8[7] = 0;
        b8 += 8;
        fb++;
    }
    
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 224, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer8);
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

#pragma mark -  OpenGL ES 2 shader compilation

- (BOOL)loadShaders
{
    GLuint vertShader, fragShader;
    NSString *vertShaderPathname, *fragShaderPathname;
    
    // Create shader program.
    _program = glCreateProgram();
    
    // Create and compile vertex shader.
    vertShaderPathname = [[NSBundle mainBundle] pathForResource:@"Shader" ofType:@"vsh"];
    if (![self compileShader:&vertShader type:GL_VERTEX_SHADER file:vertShaderPathname]) {
        NSLog(@"Failed to compile vertex shader");
        return NO;
    }
    
    // Create and compile fragment shader.
    fragShaderPathname = [[NSBundle mainBundle] pathForResource:@"Shader" ofType:@"fsh"];
    if (![self compileShader:&fragShader type:GL_FRAGMENT_SHADER file:fragShaderPathname]) {
        NSLog(@"Failed to compile fragment shader");
        return NO;
    }
    
    // Attach vertex shader to program.
    glAttachShader(_program, vertShader);
    
    // Attach fragment shader to program.
    glAttachShader(_program, fragShader);
    
    // Bind attribute locations.
    // This needs to be done prior to linking.
    glBindAttribLocation(_program, 0, "aPosition");
    glBindAttribLocation(_program, 1, "aTexCoord");
    
    // Link program.
    if (![self linkProgram:_program]) {
        NSLog(@"Failed to link program: %d", _program);
        
        if (vertShader) {
            glDeleteShader(vertShader);
            vertShader = 0;
        }
        if (fragShader) {
            glDeleteShader(fragShader);
            fragShader = 0;
        }
        if (_program) {
            glDeleteProgram(_program);
            _program = 0;
        }
        
        return NO;
    }
    
    // Release vertex and fragment shaders.
    if (vertShader) {
        glDetachShader(_program, vertShader);
        glDeleteShader(vertShader);
    }
    if (fragShader) {
        glDetachShader(_program, fragShader);
        glDeleteShader(fragShader);
    }
    
    return YES;
}

- (BOOL)compileShader:(GLuint *)shader type:(GLenum)type file:(NSString *)file
{
    GLint status;
    const GLchar *source;
    
    source = (GLchar *)[[NSString stringWithContentsOfFile:file encoding:NSUTF8StringEncoding error:nil] UTF8String];
    if (!source) {
        NSLog(@"Failed to load vertex shader");
        return NO;
    }
    
    *shader = glCreateShader(type);
    glShaderSource(*shader, 1, &source, NULL);
    glCompileShader(*shader);
    
#if defined(DEBUG)
    GLint logLength;
    glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetShaderInfoLog(*shader, logLength, &logLength, log);
        NSLog(@"Shader compile log:\n%s", log);
        free(log);
    }
#endif
    
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
    if (status == 0) {
        glDeleteShader(*shader);
        return NO;
    }
    
    return YES;
}

- (BOOL)linkProgram:(GLuint)prog
{
    GLint status;
    glLinkProgram(prog);
    
#if defined(DEBUG)
    GLint logLength;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        NSLog(@"Program link log:\n%s", log);
        free(log);
    }
#endif
    
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (status == 0) {
        return NO;
    }
    
    return YES;
}

- (BOOL)validateProgram:(GLuint)prog
{
    GLint logLength, status;
    
    glValidateProgram(prog);
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        NSLog(@"Program validate log:\n%s", log);
        free(log);
    }
    
    glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
    if (status == 0) {
        return NO;
    }
    
    return YES;
}

@end
