#version 330 core

precision highp float;

// variables shared with main.cpp
const int MAX_ORBIT_POINTS = 20;
const int MAX_PALETTE_COLORS = 20;

const int max_iterations = 2048;
uniform mat3 pixel_to_mandel;

uniform float coloring_shift;
uniform int num_palette_colors;
uniform vec4 palette[MAX_PALETTE_COLORS];

uniform bool draw_orbit_points;
uniform vec2 orbit_points[MAX_ORBIT_POINTS];
uniform int num_orbit_points;
uniform vec4 orbit_point_color;

float length_squared(vec2 vector)
{
  return vector.x * vector.x + vector.y * vector.y;
}

vec3 orbit_trap(float squared_dist)
{
  // if (iteration != max_iterations) { return vec3(0.0); }
  float n = sqrt(squared_dist);
  // return palette[(int(8.5 * n) % 3) + 1].xyz;
  // TODO try picking a hue instead of using a color palette
  // if (n < 0.01 * coloring_shift) { return palette[0].xyz; } // trapcolor // TODO add blending
  // if (n < 0.01 * coloring_shift) { return mix(palette[0].xyz, palette[1].xyz, 100*n); } // trapcolor // TODO add blending
  float indexx = (coloring_shift * n);
  float n_colors = num_palette_colors - 1;
  indexx -= int(indexx / n_colors) * n_colors;
  int col1 = int(indexx);
  int col2 = int(indexx + 1.0) % int(n_colors);
  float a = indexx - float(col1);
  return mix(palette[col1 + 1].xyz, palette[col2 + 1].xyz, a);
}

// average the color over all the orbit points
vec3 orbit_trap_multiple_points(float dist[MAX_ORBIT_POINTS])
{
  vec3 sum = vec3(0.0f);
  for (int i = 0; i < num_orbit_points; i++)
  {
    sum += orbit_trap(dist[i]);
  }
  return sum / num_orbit_points;
}

vec3 mandelbrot(vec2 coord)
{
  vec2 z = vec2(0.0);
  int iteration = 0;

  float dist_min[MAX_ORBIT_POINTS];
  float dist_max[MAX_ORBIT_POINTS];
  for (int i = 0; i < MAX_ORBIT_POINTS; i++)
  {
    dist_min[i] = 1e20f;
    dist_max[i] = 0.0f;
  }

  while (iteration < max_iterations && length(z) < 2.0)
  {
    z = vec2(z.x * z.x - z.y * z.y + coord.x, 2.0 * z.x * z.y + coord.y);
    iteration++;

    for (int i = 0; i < num_orbit_points; i++)
    {
      dist_min[i] = min(dist_min[i], length_squared(z - orbit_points[i]));
      dist_max[i] = max(dist_max[i], length_squared(z - orbit_points[i]));
    }

  }

  return orbit_trap_multiple_points(dist_min);
  // return orbit_trap_multiple_points(dist_max);

  // float n = sqrt(dist);
  // float a = 16.0;
  // vec3 col = vec3(0.0);
  // for (int i = 0; i < num_colors; i++)
  // {
  //   col += (0.5 * sin(a * n + float(i) * coloring_shift) + 0.5) * palette[i].xyz;
  // }
  // return col;

  // change the shift with the zoom level
  // vec3 loc1 = pixel_to_mandel * vec3(0.0, 0.0, 1.0);
  // vec3 loc2 = pixel_to_mandel * vec3(0.0, 720.0, 1.0);
  // loc1 = loc1 / loc1.z;
  // loc2 = loc2 / loc2.z;
  // float zoom = 20.0 * abs(loc1.y - loc2.y);
  // float indexx = (coloring_shift * n / zoom);
  
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

  if (draw_orbit_points)
  {
    vec3 max_heigth = pixel_to_mandel * vec3(0.0f, 720.0f, 1.0f);
    vec3 min_heigth = pixel_to_mandel * vec3(0.0f, 0.0f, 1.0f);
    float height = abs(max_heigth.y - min_heigth.y) / 2.0f;

    for (int i = 0; i < num_orbit_points; i++)
    {
      if (length(coords - orbit_points[i]) < 0.01 * height)
      {
        colour = orbit_point_color.xyz;
      }
    }
  }

  gl_FragColor = vec4(colour, 1.0);

  // gl_FragColor = vec4(0.0, 0.6, 1.0, 1.0);
}
