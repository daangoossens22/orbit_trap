#version 330 core

precision highp float;

const int max_iterations = 2048;
uniform mat3 pixel_to_mandel;

vec3 mandelbrot(vec2 coord)
{
  vec2 z = vec2(0.0);
  int iteration = 0;
  while (iteration < max_iterations && length(z) < 2.0)
  {
    z = vec2(z.x * z.x - z.y * z.y + coord.x, 2.0 * z.x * z.y + coord.y);
    iteration++;
  }

  if (iteration == max_iterations) { return vec3(0.0); }
  else
  {
    float n = float(iteration);
    float a = 0.1;
    float r = 0.5 * sin(a * n) + 0.5;
    float g = 0.5 * sin(a * n + 2.2222) + 0.5;
    float b = 0.5 * sin(a * n + 4.4444) + 0.5;
    return vec3(r, g, b);
  }
}

void main()
{
  vec3 coords_temp = pixel_to_mandel * vec3(gl_FragCoord.xy, 1.0);
  vec2 coords = coords_temp.xy / coords_temp.z; // remove homogeneous coordinate

  vec3 colour = mandelbrot(coords);
  gl_FragColor = vec4(colour, 1.0);

  // gl_FragColor = vec4(0.0, 0.6, 1.0, 1.0);
}
