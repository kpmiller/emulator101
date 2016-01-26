//
//  Shader.vsh
//  PhoneInvaders
//

attribute vec2 aPosition;
attribute vec2 aTexCoord;

varying mediump vec2 vTexCoord;

void main()
{
    vTexCoord   = aTexCoord;
    gl_Position = vec4(aPosition, 0.0, 1.0);
}
