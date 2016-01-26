//
//  Shader.fsh
//  PhoneInvaders
//

precision mediump float;
varying mediump vec2 vTexCoord;
uniform sampler2D tsampler;

void main()
{
    gl_FragColor = texture2D(tsampler, vTexCoord);
}
