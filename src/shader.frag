#version 330 core

precision highp float;

const int max_iterations = 2048;
uniform mat3 pixel_to_mandel;

uniform float coloring_shift;
uniform int num_colors;
uniform vec4 palette[20];

float length_squared(vec2 vector)
{
  return vector.x * vector.x + vector.y * vector.y;
}

vec3 mandelbrot(vec2 coord)
{
  vec2 z = vec2(0.0);
  int iteration = 0;
  float dist = 1e20f;
  float dist_max = 0.0;
  vec2 point = vec2(0.0);
  while (iteration < max_iterations && length(z) < 2.0)
  {
    z = vec2(z.x * z.x - z.y * z.y + coord.x, 2.0 * z.x * z.y + coord.y);
    iteration++;
    dist = min(dist, length_squared(z - point));
    dist_max = max(dist, length_squared(z - point));
    // if(length(z - point) < 0.1)
    // {
    //   vec3 cols[4];
    //   cols[0] = vec3(1.0, 0.0, 0.0);
    //   cols[1] = vec3(0.0, 1.0, 0.0);
    //   cols[2] = vec3(0.0, 0.0, 1.0);
    //   cols[3] = vec3(1.0, 1.0, 1.0);
    //   return cols[iteration % 4];
    // }
  }

  // float n = sqrt(dist);
  // float a = 16.0;
  // vec3 col = vec3(0.0);
  // for (int i = 0; i < num_colors; i++)
  // {
  //   col += (0.5 * sin(a * n + float(i) * coloring_shift) + 0.5) * palette[i].xyz;
  // }
  // return col;
  
  // if (iteration != max_iterations) { return vec3(0.0); }
  float n = sqrt(dist);
  // return palette[(int(8.5 * n) % 3) + 1].xyz;
  if (n < 0.01 * coloring_shift) { return palette[0].xyz; } // trapcolor // TODO add blending
  float indexx = (coloring_shift * n);
  float n_colors = 3.0f;
  indexx -= int(indexx / n_colors) * n_colors;
  int col1 = int(indexx);
  int col2 = int(indexx + 1.0) % int(n_colors);
  float a = indexx - float(col1);
  return mix(palette[col1 + 1].xyz, palette[col2 + 1].xyz, a);

  // float n = sqrt(dist);
  // float a = 16.0;
  // float r = 0.5 * sin(a * n) + 0.5;
  // float g = 0.5 * sin(a * n + 1.1111) + 0.5;
  // float b = 0.5 * sin(a * n + 2.2222) + 0.5;
  // return vec3(r, g, b);

  // orbit trap with sin
  // float n = sqrt(dist);
  // float a = 2.1;
  // float r = 0.5 * sin(a * n) + 0.5;
  // float g = 0.5 * sin(a * n + 2.2222) + 0.5;
  // float b = 0.5 * sin(a * n + 4.4444) + 0.5;
  // return vec3(r, g, b);

  // if (iteration == max_iterations)
  // {
  //   // float dist = sqrt(dist);
  //   // return vec3(dist);
  //   return vec3(0.0);
  //   // float n = sqrt(dist);
  //   // float a = 4.1;
  //   // float r = 0.5 * sin(a * n) + 0.5;
  //   // float g = 0.5 * sin(a * n + 2.2222) + 0.5;
  //   // float b = 0.5 * sin(a * n + 4.4444) + 0.5;
  //   // return vec3(r, g, b);
  // }
  // else
  // {
  //   float n = sqrt(dist);
  //   float a = 4.1;
  //   float r = 0.5 * sin(a * n) + 0.5;
  //   float g = 0.5 * sin(a * n + 2.2222) + 0.5;
  //   float b = 0.5 * sin(a * n + 4.4444) + 0.5;
  //   return vec3(r, g, b);
  //   // return vec3(0.0);
  // }

  // cool coloring
  // if (iteration == max_iterations) { return vec3(0.0); }
  // else
  // {
  //   float n = float(iteration);
  //   float a = 0.1;
  //   float r = 0.5 * sin(a * n) + 0.5;
  //   float g = 0.5 * sin(a * n + 2.2222) + 0.5;
  //   float b = 0.5 * sin(a * n + 4.4444) + 0.5;
  //   return vec3(r, g, b);
  // }
}

void main()
{
  vec3 coords_temp = pixel_to_mandel * vec3(gl_FragCoord.xy, 1.0);
  vec2 coords = coords_temp.xy / coords_temp.z; // remove homogeneous coordinate

  vec3 colour = mandelbrot(coords);
  gl_FragColor = vec4(colour, 1.0);

  // gl_FragColor = vec4(0.0, 0.6, 1.0, 1.0);
}
