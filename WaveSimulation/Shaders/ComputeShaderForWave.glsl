#version 430

uniform float WaveFactor;

layout(local_size_x = 10, local_size_y = 10) in;

struct Attributes
{
   vec3 Vertex;
   //vec3 Normal;
   vec2 TexCoord;
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
   float updated = 2.0f * Pn[index].Vertex.y - Pn_prev[index].Vertex.y;

   if (gl_GlobalInvocationID.y < points.y - 1) {
      updated += WaveFactor * Pn[index + points.x].Vertex.y;
   }
   if (gl_GlobalInvocationID.y > 0) {
      updated += WaveFactor * Pn[index - points.x].Vertex.y;
   }
   if (gl_GlobalInvocationID.x > 0) {
      updated += WaveFactor * Pn[index - 1].Vertex.y;
   }
   if (gl_GlobalInvocationID.x < points.x - 1) {
      updated += WaveFactor * Pn[index + 1].Vertex.y;
   }

   updated /= (1.0f + 4.0f * WaveFactor);
   Pn_next[index].Vertex = vec3(Pn[index].Vertex.x, updated, Pn[index].Vertex.z);
   Pn_next[index].TexCoord = Pn[index].TexCoord;
}


/*layout(binding = 0, std430) buffer PrevPoints {
   vec3 Pn_prev[];
};

layout(binding = 1, std430) buffer CurrPoints {
   vec3 Pn[];
};

layout(binding = 2, std430) buffer NextPoints {
   vec3 Pn_next[];
};

void main() 
{
   uvec3 points = gl_NumWorkGroups * gl_WorkGroupSize;
   uint index = gl_GlobalInvocationID.y * points.x + gl_GlobalInvocationID.x;
   float updated = 2.0f * Pn[index].y - Pn_prev[index].y;

   if (gl_GlobalInvocationID.y < points.y - 1) {
      updated += WaveFactor * Pn[index + points.x].y;
   }
   if (gl_GlobalInvocationID.y > 0) {
      updated += WaveFactor * Pn[index - points.x].y;
   }
   if (gl_GlobalInvocationID.x > 0) {
      updated += WaveFactor * Pn[index - 1].y;
   }
   if (gl_GlobalInvocationID.x < points.x - 1) {
      updated += WaveFactor * Pn[index + 1].y;
   }

   updated /= (1.0f + 4.0f * WaveFactor);
   Pn_next[index] = vec3(Pn[index].x, updated, Pn[index].z);
}*/