#version 430

uniform float WaveFactor;

layout(local_size_x = 10, local_size_y = 10) in;

struct Attributes
{
   float x, y, z, nx, ny, nz, s, t;
};

layout(binding = 0, std430) buffer PrevPoints {
   Attributes Pn_prev[];
};

layout(binding = 1, std430) buffer CurrPoints {
   Attributes Pn[];
};

layout(binding = 2, std430) buffer NextPoints {
   Attributes Pn_next[];
};

void main() 
{
   uvec3 points = gl_NumWorkGroups * gl_WorkGroupSize;
   uint index = gl_GlobalInvocationID.y * points.x + gl_GlobalInvocationID.x;

   float updated_height = 2.0f * Pn[index].y - Pn_prev[index].y;
   vec3 estimated_normal = vec3(0.0f, 0.0f, 0.0f);

   if (0 < gl_GlobalInvocationID.x) {
      updated_height += WaveFactor * Pn[index - 1].y;
   }
   if (gl_GlobalInvocationID.x < points.x - 1) {
      updated_height += WaveFactor * Pn[index + 1].y;
   }
   if (0 < gl_GlobalInvocationID.y) {
      updated_height += WaveFactor * Pn[index - points.x].y;
   }
   if (gl_GlobalInvocationID.y < points.y - 1) {
      updated_height += WaveFactor * Pn[index + points.x].y;
   }

   updated_height /= (1.0f + 4.0f * WaveFactor);
   Pn_next[index].x = Pn[index].x;
   Pn_next[index].y = updated_height;
   Pn_next[index].z = Pn[index].z;
   Pn_next[index].s = Pn[index].s;
   Pn_next[index].t = Pn[index].t;
}