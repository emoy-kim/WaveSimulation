#version 430

layout(local_size_x = 10, local_size_y = 10) in;

struct Attributes
{
   float x, y, z, nx, ny, nz, s, t;
};

layout(binding = 0, std430) buffer InOutPoints {
   Attributes Pn[];
};

void main() 
{
   uvec3 points = gl_NumWorkGroups * gl_WorkGroupSize;
   uint index = gl_GlobalInvocationID.y * points.x + gl_GlobalInvocationID.x;

   vec3 estimated_normal = vec3(0.0f, 0.0f, 0.0f);
   vec3 point_vec = vec3(Pn[index].x, Pn[index].y, Pn[index].z);

   if (0 < gl_GlobalInvocationID.y) {
      uint top_index = index - points.x;
      vec3 top_vec = vec3(Pn[top_index].x, Pn[top_index].y, Pn[top_index].z) - point_vec;

      if (0 < gl_GlobalInvocationID.x) {
         vec3 left_vec = vec3(Pn[index - 1].x, Pn[index - 1].y, Pn[index - 1].z) - point_vec;
         vec3 top_left_vec = vec3(Pn[top_index - 1].x, Pn[top_index - 1].y, Pn[top_index - 1].z) - point_vec;
         estimated_normal += cross( top_vec, top_left_vec );
         estimated_normal += cross( top_left_vec, left_vec );
      }
      if (gl_GlobalInvocationID.x < points.x - 1) {
         vec3 right_vec = vec3(Pn[index + 1].x, Pn[index + 1].y, Pn[index + 1].z) - point_vec;
         vec3 top_right_vec = vec3(Pn[top_index + 1].x, Pn[top_index + 1].y, Pn[top_index + 1].z) - point_vec;
         estimated_normal += cross( right_vec, top_right_vec );
         estimated_normal += cross( top_right_vec, top_vec );
      }
   }

   if (gl_GlobalInvocationID.y < points.y - 1) {
      uint bottom_index = index + points.x;
      vec3 bottom_vec = vec3(Pn[bottom_index].x, Pn[bottom_index].y, Pn[bottom_index].z) - point_vec;

      if (0 < gl_GlobalInvocationID.x) {
         vec3 left_vec = vec3(Pn[index - 1].x, Pn[index - 1].y, Pn[index - 1].z) - point_vec;
         vec3 bottom_left_vec = vec3(Pn[bottom_index - 1].x, Pn[bottom_index - 1].y, Pn[bottom_index - 1].z) - point_vec;
         estimated_normal += cross( left_vec, bottom_left_vec );
         estimated_normal += cross( bottom_left_vec, bottom_vec );
      }
      if (gl_GlobalInvocationID.x < points.x - 1) {
         vec3 right_vec = vec3(Pn[index + 1].x, Pn[index + 1].y, Pn[index + 1].z) - point_vec;
         vec3 bottom_right_vec = vec3(Pn[bottom_index + 1].x, Pn[bottom_index + 1].y, Pn[bottom_index + 1].z) - point_vec;
         estimated_normal += cross( bottom_vec, bottom_right_vec );
         estimated_normal += cross( bottom_right_vec, right_vec );
      }
   }

   estimated_normal = normalize( estimated_normal );
   Pn[index].nx = estimated_normal.x;
   Pn[index].ny = estimated_normal.y;
   Pn[index].nz = estimated_normal.z;
}