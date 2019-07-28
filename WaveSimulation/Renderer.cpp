#include "Renderer.h"

RendererGL* RendererGL::Renderer = nullptr;
RendererGL::RendererGL() : 
   Window( nullptr ), ClickedPoint( -1.0f, -1.0f ), WaveTargetIndex( 0 ), WaveFactor( 3.0f ),
   WavePointNumSize( 100, 100 ), WaveGridSize( 5, 5 )
{
   Renderer = this;

   initialize();
   printOpenGLInformation();
}

RendererGL::~RendererGL()
{
   glfwTerminate();
}

void RendererGL::printOpenGLInformation() const
{
   cout << "****************************************************************" << endl;
   cout << " - GLFW version supported: " << glfwGetVersionString() << endl;
   cout << " - GLEW version supported: " << glewGetString( GLEW_VERSION ) << endl;
   cout << " - OpenGL renderer: " << glGetString( GL_RENDERER ) << endl;
   cout << " - OpenGL version supported: " << glGetString( GL_VERSION ) << endl;
   cout << " - OpenGL shader version supported: " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << endl  ;
   cout << "****************************************************************" << endl << endl;
}

void RendererGL::initialize()
{
   if (!glfwInit()) {
      cout << "Cannot Initialize OpenGL..." << endl;
      return;
   }
   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
   glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

   const int width = 1920;
   const int height = 1080;
   Window = glfwCreateWindow( width, height, "Main Camera", nullptr, nullptr );
   glfwMakeContextCurrent( Window );
   glewInit();
   
   registerCallbacks();
   
   glEnable( GL_DEPTH_TEST );
   glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );

   MainCamera.updateWindowSize( width, height );
   ObjectShader.setShader(
      "Shaders/VertexShaderForObject.glsl",
      "Shaders/FragmentShaderForObject.glsl"
   );
   ObjectShader.setComputeShaders( { "Shaders/ComputeShaderForWave.glsl" } );
}

void RendererGL::error(int error, const char* description) const
{
   puts( description );
}

void RendererGL::errorWrapper(int error, const char* description)
{
   Renderer->error( error, description );
}

void RendererGL::cleanup(GLFWwindow* window)
{
   glDeleteProgram( ObjectShader.ShaderProgram );
   glDeleteVertexArrays( 1, &WaveObject.ObjVAO );
   glDeleteBuffers( 1, &WaveObject.ObjVBO );

   glfwSetWindowShouldClose( window, GLFW_TRUE );
}

void RendererGL::cleanupWrapper(GLFWwindow* window)
{
   Renderer->cleanup( window );
}

void RendererGL::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   if (action != GLFW_PRESS) return;

   switch (key) {
      case GLFW_KEY_UP:
         MainCamera.moveForward();
         break;
      case GLFW_KEY_DOWN:
         MainCamera.moveBackward();
         break;
      case GLFW_KEY_LEFT:
         MainCamera.moveLeft();
         break;
      case GLFW_KEY_RIGHT:
         MainCamera.moveRight();
         break;
      case GLFW_KEY_W:
         MainCamera.moveUp();
         break;
      case GLFW_KEY_S:
         MainCamera.moveDown();
         break;
      case GLFW_KEY_I:
         MainCamera.resetCamera();
         break;
      case GLFW_KEY_L:
         Lights.toggleLightSwitch();
         cout << "Light Turned " << (Lights.isLightOn() ? "On!" : "Off!") << endl;
         break;
      case GLFW_KEY_P:
         cout << "Camera Position: " << 
            MainCamera.CamPos.x << ", " << MainCamera.CamPos.y << ", " << MainCamera.CamPos.z << endl;
         break;
      case GLFW_KEY_Q:
      case GLFW_KEY_ESCAPE:
         cleanupWrapper( window );
         break;
      default:
         return;
   }
}

void RendererGL::keyboardWrapper(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   Renderer->keyboard( window, key, scancode, action, mods );
}

void RendererGL::cursor(GLFWwindow* window, double xpos, double ypos)
{
   if (MainCamera.getMovingState()) {
      const int dx = static_cast<int>(round( xpos ) - ClickedPoint.x);
      const int dy = static_cast<int>(round( ypos ) - ClickedPoint.y);
      MainCamera.pitch( dy );
      MainCamera.yaw( dx );
   }
}

void RendererGL::cursorWrapper(GLFWwindow* window, double xpos, double ypos)
{
   Renderer->cursor( window, xpos, ypos );
}

void RendererGL::mouse(GLFWwindow* window, int button, int action, int mods)
{
   if (button == GLFW_MOUSE_BUTTON_LEFT) {
      const bool moving_state = action == GLFW_PRESS;
      if (moving_state) {
         double x, y;
         glfwGetCursorPos( window, &x, &y );
         ClickedPoint.x = static_cast<float>(round( x ));
         ClickedPoint.y = static_cast<float>(round( y ));
      }
      MainCamera.setMovingState( moving_state );
   }
}

void RendererGL::mouseWrapper(GLFWwindow* window, int button, int action, int mods)
{
   Renderer->mouse( window, button, action, mods );
}

void RendererGL::mousewheel(GLFWwindow* window, double xoffset, double yoffset)
{
   if (yoffset >= 0.0) {
      MainCamera.zoomIn();
   }
   else {
      MainCamera.zoomOut();
   }
}

void RendererGL::mousewheelWrapper(GLFWwindow* window, double xoffset, double yoffset)
{
   Renderer->mousewheel( window, xoffset, yoffset );
}

void RendererGL::reshape(GLFWwindow* window, int width, int height)
{
   MainCamera.updateWindowSize( width, height );
   glViewport( 0, 0, width, height );
}

void RendererGL::reshapeWrapper(GLFWwindow* window, int width, int height)
{
   Renderer->reshape( window, width, height );
}

void RendererGL::registerCallbacks() const
{
   glfwSetErrorCallback( errorWrapper );
   glfwSetWindowCloseCallback( Window, cleanupWrapper );
   glfwSetKeyCallback( Window, keyboardWrapper );
   glfwSetCursorPosCallback( Window, cursorWrapper );
   glfwSetMouseButtonCallback( Window, mouseWrapper );
   glfwSetScrollCallback( Window, mousewheelWrapper );
   glfwSetFramebufferSizeCallback( Window, reshapeWrapper );
}

void RendererGL::setLights()
{  
   vec4 light_position(10.0f, 10.0f, -10.0f, 1.0f);
   vec4 ambient_color(0.3f, 0.3f, 0.3f, 1.0f);
   vec4 diffuse_color(0.7f, 0.7f, 0.7f, 1.0f);
   vec4 specular_color(0.9f, 0.9f, 0.9f, 1.0f);
   //Lights.addLight( light_position, ambient_color, diffuse_color, specular_color );

   light_position = vec4(3.0f, 10.0f, 3.0f, 1.0f);
   ambient_color = vec4(0.2f, 0.2f, 0.2f, 1.0f);
   diffuse_color = vec4(0.9f, 0.5f, 0.1f, 1.0f);
   specular_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
   vec3 spotlight_direction(0.0f, -1.0f, 0.0f);
   float spotlight_exponent = 128;
   float spotlight_cutoff_angle_in_degree = 7.0f;
   Lights.addLight( 
      light_position, 
      ambient_color, 
      diffuse_color, 
      specular_color,
      spotlight_direction,
      spotlight_exponent,
      spotlight_cutoff_angle_in_degree
   );  
}

void RendererGL::setWaveObject()
{
   if (WaveObject.ObjVAO != 0) {
      glDeleteVertexArrays( 1, &WaveObject.ObjVAO );
      glDeleteBuffers( 1, &WaveObject.ObjVBO );
   }

   const float ds = 1.0f / static_cast<float>(WavePointNumSize.x - 1);
   const float dt = 1.0f / static_cast<float>(WavePointNumSize.y - 1);
   const float dx = static_cast<float>(WaveGridSize.x) * ds;
   const float dy = static_cast<float>(WaveGridSize.y) * dt;
   const auto mid_x = static_cast<float>(WavePointNumSize.x >> 1);
   const auto mid_y = static_cast<float>(WavePointNumSize.y >> 1);
   
   const float initial_radius_squared = 64.0f;
   const float initial_wave_factor = glm::pi<float>() / initial_radius_squared;
   const float initial_wave_height = 0.7f;

   vector<vec3> wave_vertices, wave_normals;
   vector<vec2> wave_textures;
   for (int j = 0; j < WavePointNumSize.y; ++j) {
      const auto y = static_cast<float>(j);
      for (int i = 0; i < WavePointNumSize.x; ++i) {
         const auto x = static_cast<float>(i);   
         wave_vertices.emplace_back( x * dx, 0.0f, y * dy );

         const float distance_squared = (x - mid_x) * (x - mid_x) + (y - mid_y) * (y - mid_y);
         if (distance_squared <= initial_radius_squared) {
            const float theta = sqrt( initial_wave_factor * distance_squared );
            wave_vertices.back().y = initial_wave_height * (cos( theta ) + 1.0f);
         }

         wave_normals.emplace_back( 0.0f, 0.0f, 0.0f );
         wave_textures.emplace_back( x * ds, y * dt );
      }
   }
   
   WaveObject.setObject( GL_TRIANGLE_STRIP, wave_vertices, wave_normals, wave_textures, "water.png" );

   WaveObject.prepareShaderStorageBuffer();
   
   vector<GLuint> indices;
   for (int j = 0; j < WavePointNumSize.y - 1; ++j) {
      for (int i = 0; i < WavePointNumSize.x; ++i) {
         indices.emplace_back( (j + 1) * WavePointNumSize.x + i );
         indices.emplace_back( j * WavePointNumSize.x + i );
      }
   }
   WaveObject.addBufferObject( GL_ELEMENT_ARRAY_BUFFER, indices, GL_DYNAMIC_COPY );

   const vec4 diffuse_color = { 0.0f, 0.47f, 0.75f, 1.0f };
   WaveObject.setDiffuseReflectionColor( diffuse_color );

   const float delta_time = 0.003f;
   WaveFactor = WaveFactor * WaveFactor * delta_time * delta_time / dx;
}

void RendererGL::drawWaveObject()
{
   glUseProgram( ObjectShader.ComputeShaderPrograms[0] );
   glUniform1f( ObjectShader.CustomLocations["WaveFactor"], WaveFactor );
   
   glDispatchCompute( WavePointNumSize.x / 10, WavePointNumSize.y / 10, 1 );
   glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
   
   glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, WaveObject.ShaderStorageBufferObjects[WaveTargetIndex] );
   glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, WaveObject.ShaderStorageBufferObjects[(WaveTargetIndex + 1) % 3] );
   glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 2, WaveObject.ShaderStorageBufferObjects[(WaveTargetIndex + 2) % 3] );
   WaveTargetIndex = (WaveTargetIndex + 1) % 3;

   glUseProgram( ObjectShader.ShaderProgram );

   mat4 to_world = mat4(1.0f);
   const mat4 model_view_projection = MainCamera.ProjectionMatrix * MainCamera.ViewMatrix * to_world;
   glUniformMatrix4fv( ObjectShader.Location.World, 1, GL_FALSE, &to_world[0][0] );
   glUniformMatrix4fv( ObjectShader.Location.View, 1, GL_FALSE, &MainCamera.ViewMatrix[0][0] );
   glUniformMatrix4fv( ObjectShader.Location.Projection, 1, GL_FALSE, &MainCamera.ProjectionMatrix[0][0] );
   glUniformMatrix4fv( ObjectShader.Location.ModelViewProjection, 1, GL_FALSE, &model_view_projection[0][0] );

   WaveObject.transferUniformsToShader( ObjectShader );
   Lights.transferUniformsToShader( ObjectShader );
   
   glBindVertexArray( WaveObject.ObjVAO );
   for (int j = 0; j < WavePointNumSize.y - 1; ++j) {
      glDrawElements( 
         WaveObject.DrawMode, 
         WavePointNumSize.x * 2, 
         GL_UNSIGNED_INT, 
         reinterpret_cast<GLvoid *>(j * WavePointNumSize.x * 2 * sizeof GLuint)
      );
   }
}

void RendererGL::render()
{
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   drawWaveObject();

   glBindVertexArray( 0 );
   glUseProgram( 0 );
}

void RendererGL::update()
{
   
}

void RendererGL::play()
{
   if (glfwWindowShouldClose( Window )) initialize();

   setLights();
   setWaveObject();
   ObjectShader.setUniformLocations( Lights.TotalLightNum );
   ObjectShader.addUniformLocation( ObjectShader.ComputeShaderPrograms[0], "WaveFactor" );

   const double update_time = 0.1;
   double last = glfwGetTime(), time_delta = 0.0;
   while (!glfwWindowShouldClose( Window )) {
      const double now = glfwGetTime();
      time_delta += now - last;
      last = now;
      if (time_delta >= update_time) {
         update();
         time_delta -= update_time;
      }

      render();

      glfwPollEvents();
      glfwSwapBuffers( Window );
   }
   glfwDestroyWindow( Window );
}