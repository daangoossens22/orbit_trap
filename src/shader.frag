#version 330 core

precision highp float;

const int max_iterations = 100;

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
  // x = [-2.5, 1.0]
  // y = [-1.0, 1.0]
  vec2 coords = vec2(gl_FragCoord.x / 1280.0, gl_FragCoord.y / 720.0);
  coords.x *= 3.5;
  coords.x -= 2.5;
  coords.y *= 2.0;
  coords.y -= 1.0;
  vec3 val = mandelbrot(coords);
  gl_FragColor = vec4(val, 1.0);

  // gl_FragColor = vec4(0.0, 0.6, 1.0, 1.0);
}
