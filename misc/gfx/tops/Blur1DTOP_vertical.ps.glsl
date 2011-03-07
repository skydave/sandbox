varying vec2 uv;
uniform sampler2D input;

uniform float pixelStep; // 1.0/yres

void main(void)
{
   vec4 sum = vec4(0.0);

   // blur in y (vertical)
   // take nine samples, with the distance blurSize between them
   sum += texture2D(input, vec2(uv.x, uv.y - 4.0*pixelStep)) * 0.05;
   sum += texture2D(input, vec2(uv.x, uv.y - 3.0*pixelStep)) * 0.09;
   sum += texture2D(input, vec2(uv.x, uv.y - 2.0*pixelStep)) * 0.12;
   sum += texture2D(input, vec2(uv.x, uv.y - pixelStep)) * 0.15;
   sum += texture2D(input, vec2(uv.x, uv.y)) * 0.16;
   sum += texture2D(input, vec2(uv.x, uv.y + pixelStep)) * 0.15;
   sum += texture2D(input, vec2(uv.x, uv.y + 2.0*pixelStep)) * 0.12;
   sum += texture2D(input, vec2(uv.x, uv.y + 3.0*pixelStep)) * 0.09;
   sum += texture2D(input, vec2(uv.x, uv.y + 4.0*pixelStep)) * 0.05;

   gl_FragColor = sum;
}
