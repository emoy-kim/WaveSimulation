#include "Renderer.h"

RendererGL::RendererGL() : 
   Window( nullptr ), FrameWidth( 1920 ), FrameHeight( 1080 ), WaveTargetIndex( 0 ), WaveFactor( 5.0f ),
   WavePointNumSize( 100, 100 ), WaveGridSize( 5, 5 ), ClickedPoint( -1, -1 ),
   MainCamera( std::make_unique<CameraGL>() ), ObjectShader( std::make_unique<ShaderGL>() ),
   WaveObject( std::make_unique<ObjectGL>() ), Lights( std::make_unique<LightGL>() )
{
   Renderer = this;

   initialize();
   printOpenGLInformation();
}

RendererGL::~RendererGL()
{
   glfwTerminate();
}

void RendererGL::printOpenGLInformation()
{
   std::cout << "****************************************************************\n";
   std::cout << " - GLFW version supported: " << glfwGetVersionString() << "\n";
   std::cout << " - OpenGL renderer: " << glGetString( GL_RENDERER ) << "\n";
   std::cout << " - OpenGL version supported: " << glGetString( GL_VERSION ) << "\n";
   std::cout << " - OpenGL shader version supported: " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << "\n";
   std::cout << "****************************************************************\n\n";
}

void RendererGL::initialize()
{
   if (!glfwInit()) {
      std::cout << "Cannot Initialize OpenGL...\n";
      return;
   }
   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
   glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

   Window = glfwCreateWindow( FrameWidth, FrameHeight, "Main Camera", nullptr, nullptr );
   glfwMakeContextCurrent( Window );

   if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return;
   }
   
   registerCallbacks();
   
   glEnable( GL_DEPTH_TEST );
   glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );

   MainCamera->updateWindowSize( FrameWidth, FrameHeight );

   const std::string shader_directory_path = std::string(CMAKE_SOURCE_DIR) + "/shaders";
   ObjectShader->setShader(
      std::string(shader_directory_path + "/BasicPipeline.vert").c_str(),
      std::string(shader_directory_path + "/BasicPipeline.frag").c_str()
   );
   ObjectShader->setComputeShaders( { 
      std::string(shader_directory_path + "/Wave.comp").c_str(),
      std::string(shader_directory_path + "/WaveNormal.comp").c_str()
   } );
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
         MainCamera->moveForward();
         break;
      case GLFW_KEY_DOWN:
         MainCamera->moveBackward();
         break;
      case GLFW_KEY_LEFT:
         MainCamera->moveLeft();
         break;
      case GLFW_KEY_RIGHT:
         MainCamera->moveRight();
         break;
      case GLFW_KEY_W:
         MainCamera->moveUp();
         break;
      case GLFW_KEY_S:
         MainCamera->moveDown();
         break;
      case GLFW_KEY_I:
         MainCamera->resetCamera();
         break;
      case GLFW_KEY_L:
         Lights->toggleLightSwitch();
         std::cout << "Light Turned " << (Lights->isLightOn() ? "On!\n" : "Off!\n");
         break;
      case GLFW_KEY_P: {
         const glm::vec3 pos = MainCamera->getCameraPosition();
         std::cout << "Camera Position: " << pos.x << ", " << pos.y << ", " << pos.z << "\n";
      } break;
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
   if (MainCamera->getMovingState()) {
      const auto x = static_cast<int>(round( xpos ));
      const auto y = static_cast<int>(round( ypos ));
      const int dx = x - ClickedPoint.x;
      const int dy = y - ClickedPoint.y;
      MainCamera->moveForward( -dy );
      MainCamera->rotateAroundWorldY( -dx );

      if (glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_RIGHT ) == GLFW_PRESS) {
         MainCamera->pitch( -dy );
      }

      ClickedPoint.x = x;
      ClickedPoint.y = y;
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
         ClickedPoint.x = static_cast<int>(round( x ));
         ClickedPoint.y = static_cast<int>(round( y ));
      }
      MainCamera->setMovingState( moving_state );
   }
}

void RendererGL::mouseWrapper(GLFWwindow* window, int button, int action, int mods)
{
   Renderer->mouse( window, button, action, mods );
}

void RendererGL::mousewheel(GLFWwindow* window, double xoffset, double yoffset) const
{
   if (yoffset >= 0.0) MainCamera->zoomIn();
   else MainCamera->zoomOut();
}

void RendererGL::mousewheelWrapper(GLFWwindow* window, double xoffset, double yoffset)
{
   Renderer->mousewheel( window, xoffset, yoffset );
}

void RendererGL::reshape(GLFWwindow* window, int width, int height) const
{
   MainCamera->updateWindowSize( width, height );
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

void RendererGL::setLights() const
{  
   glm::vec4 light_position(2.0f, 150.0f, 2.0f, 1.0f);
   glm::vec4 ambient_color(0.9f, 0.9f, 0.9f, 1.0f);
   glm::vec4 diffuse_color(0.9f, 0.9f, 0.9f, 1.0f);
   glm::vec4 specular_color(0.9f, 0.9f, 0.9f, 1.0f);
   Lights->addLight( light_position, ambient_color, diffuse_color, specular_color );

   light_position = glm::vec4(2.5f, 100.0f, 2.5f, 1.0f);
   ambient_color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
   diffuse_color = glm::vec4(0.0f, 0.47f, 0.75f, 1.0f);
   specular_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
   glm::vec3 spotlight_direction(0.0f, -1.0f, 0.0f);
   float spotlight_exponent = 128;
   float spotlight_cutoff_angle_in_degree = 7.0f;
   Lights->addLight( 
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
   const float ds = 1.0f / static_cast<float>(WavePointNumSize.x - 1);
   const float dt = 1.0f / static_cast<float>(WavePointNumSize.y - 1);
   const float dx = static_cast<float>(WaveGridSize.x) * ds;
   const float dy = static_cast<float>(WaveGridSize.y) * dt;
   const auto mid_x = static_cast<float>(WavePointNumSize.x >> 1);
   const auto mid_y = static_cast<float>(WavePointNumSize.y >> 1);
   
   const float initial_radius_squared = 81.0f;
   const float initial_wave_factor = glm::pi<float>() / initial_radius_squared;
   const float initial_wave_height = 0.5f;

   std::vector<glm::vec3> wave_vertices, wave_normals;
   std::vector<glm::vec2> wave_textures;
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

   const std::string sample_directory_path = std::string(CMAKE_SOURCE_DIR) + "/samples";
   WaveObject->setObject( 
      GL_TRIANGLE_STRIP, 
      wave_vertices, 
      wave_normals, 
      wave_textures, 
      std::string(sample_directory_path + "/water.png")
   );
   WaveObject->prepareShaderStorageBuffer();
   
   std::vector<GLuint> indices;
   for (int j = 0; j < WavePointNumSize.y - 1; ++j) {
      for (int i = 0; i < WavePointNumSize.x; ++i) {
         indices.emplace_back( (j + 1) * WavePointNumSize.x + i );
         indices.emplace_back( j * WavePointNumSize.x + i );
      }
   }
   WaveObject->setElementBuffer( indices );

   const glm::vec4 diffuse_color = { 0.0f, 0.47f, 0.75f, 1.0f };
   WaveObject->setDiffuseReflectionColor( diffuse_color );

   const float delta_time = 0.0009f;
   WaveFactor = WaveFactor * WaveFactor * delta_time * delta_time / dx;
}

void RendererGL::drawWaveObject()
{
   glUseProgram( ObjectShader->getComputeShaderProgram( 0 ) );
   glUniform1f( ObjectShader->getLocation( "WaveFactor" ), WaveFactor );
   
   glDispatchCompute( WavePointNumSize.x / 10, WavePointNumSize.y / 10, 1 );
   glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
   
   glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, WaveObject->getShaderStorageBuffer( WaveTargetIndex ) );
   glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, WaveObject->getShaderStorageBuffer( (WaveTargetIndex + 1) % 3 ) );
   glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 2, WaveObject->getShaderStorageBuffer( (WaveTargetIndex + 2) % 3 ) );
   WaveTargetIndex = (WaveTargetIndex + 1) % 3;

   glUseProgram( ObjectShader->getComputeShaderProgram( 1 ) );

   glDispatchCompute( WavePointNumSize.x / 10, WavePointNumSize.y / 10, 1 );
   glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );

   glUseProgram( ObjectShader->getShaderProgram() );

   ObjectShader->transferBasicTransformationUniforms( glm::mat4(1.0f), MainCamera.get(), true );
   WaveObject->transferUniformsToShader( ObjectShader.get() );
   Lights->transferUniformsToShader( ObjectShader.get() );

   glBindTextureUnit( 0, WaveObject->getTextureID( 0 ) );
   glBindVertexArray( WaveObject->getVAO() );
   for (int j = 0; j < WavePointNumSize.y - 1; ++j) {
      glDrawElements( 
         WaveObject->getDrawMode(), 
         WavePointNumSize.x * 2, 
         GL_UNSIGNED_INT, 
         reinterpret_cast<GLvoid *>(j * WavePointNumSize.x * 2 * sizeof( GLuint ))
      );
   }
}

void RendererGL::render()
{
   glClear( OPENGL_COLOR_BUFFER_BIT | OPENGL_DEPTH_BUFFER_BIT );

   drawWaveObject();

   glBindVertexArray( 0 );
   glUseProgram( 0 );
}

void RendererGL::play()
{
   if (glfwWindowShouldClose( Window )) initialize();

   setLights();
   setWaveObject();
   ObjectShader->setUniformLocations( Lights->getTotalLightNum() );
   ObjectShader->addUniformLocationToComputeShader( "WaveFactor", 0 );

   while (!glfwWindowShouldClose( Window )) {
      render();
      
      glfwPollEvents();
      glfwSwapBuffers( Window );
   }
   glfwDestroyWindow( Window );
}