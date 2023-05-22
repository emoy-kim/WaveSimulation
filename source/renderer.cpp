#include "renderer.h"

RendererGL::RendererGL() : 
   Window( nullptr ), FrameWidth( 1920 ), FrameHeight( 1080 ), ActiveLightIndex( 0 ), WaveTargetIndex( 0 ),
   WavePointNumSize( 100, 100 ), WaveGridSize( 5, 5 ), ClickedPoint( -1, -1 ),
   MainCamera( std::make_unique<CameraGL>() ), ObjectShader( std::make_unique<ShaderGL>() ),
   WaveShader( std::make_unique<ShaderGL>() ), WaveNormalShader( std::make_unique<ShaderGL>() ),
   WaveObject( std::make_unique<ObjectGL>() ), Lights( std::make_unique<LightGL>() )
{
   Renderer = this;

   initialize();
   printOpenGLInformation();
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
   glfwSetWindowUserPointer( Window, this );
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
      std::string(shader_directory_path + "/screen.vert").c_str(),
      std::string(shader_directory_path + "/screen.frag").c_str()
   );
   WaveShader->setComputeShaders( std::string(shader_directory_path + "/wave.comp").c_str() );
   WaveNormalShader->setComputeShaders( std::string(shader_directory_path + "/wave_normal.comp").c_str() );
}

void RendererGL::cleanup(GLFWwindow* window)
{
   auto renderer = reinterpret_cast<RendererGL*>(glfwGetWindowUserPointer( window ));
   glfwSetWindowShouldClose( renderer->Window, GLFW_TRUE );
}

void RendererGL::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   if (action != GLFW_PRESS) return;

   switch (key) {
      case GLFW_KEY_L:
         Renderer->Lights->toggleLightSwitch();
         std::cout << "Light Turned " << (Renderer->Lights->isLightOn() ? "On!\n" : "Off!\n");
         break;
      case GLFW_KEY_P: {
         const glm::vec3 pos = Renderer->MainCamera->getCameraPosition();
         std::cout << "Camera Position: " << pos.x << ", " << pos.y << ", " << pos.z << "\n";
      } break;
      case GLFW_KEY_Q:
      case GLFW_KEY_ESCAPE:
         cleanup( window );
         break;
      default:
         return;
   }
}

void RendererGL::cursor(GLFWwindow* window, double xpos, double ypos)
{
   if (Renderer->MainCamera->getMovingState()) {
      const auto x = static_cast<int>(std::round( xpos ));
      const auto y = static_cast<int>(std::round( ypos ));
      const int dx = x - Renderer->ClickedPoint.x;
      const int dy = y - Renderer->ClickedPoint.y;
      Renderer->MainCamera->moveForward( -dy );
      Renderer->MainCamera->rotateAroundWorldY( -dx );

      auto renderer = reinterpret_cast<RendererGL*>(glfwGetWindowUserPointer( window ));
      if (glfwGetMouseButton( renderer->Window, GLFW_MOUSE_BUTTON_RIGHT ) == GLFW_PRESS) {
         Renderer->MainCamera->pitch( -dy );
      }

      Renderer->ClickedPoint.x = x;
      Renderer->ClickedPoint.y = y;
   }
}

void RendererGL::mouse(GLFWwindow* window, int button, int action, int mods)
{
   if (button == GLFW_MOUSE_BUTTON_LEFT) {
      const bool moving_state = action == GLFW_PRESS;
      if (moving_state) {
         double x, y;
         auto renderer = reinterpret_cast<RendererGL*>(glfwGetWindowUserPointer( window ));
         glfwGetCursorPos( renderer->Window, &x, &y );
         Renderer->ClickedPoint.x = static_cast<int>(std::round( x ));
         Renderer->ClickedPoint.y = static_cast<int>(std::round( y ));
      }
      Renderer->MainCamera->setMovingState( moving_state );
   }
}

void RendererGL::mousewheel(GLFWwindow* window, double xoffset, double yoffset)
{
   if (yoffset >= 0.0) Renderer->MainCamera->zoomIn();
   else Renderer->MainCamera->zoomOut();
}

void RendererGL::registerCallbacks() const
{
   glfwSetWindowCloseCallback( Window, cleanup );
   glfwSetKeyCallback( Window, keyboard );
   glfwSetCursorPosCallback( Window, cursor );
   glfwSetMouseButtonCallback( Window, mouse );
   glfwSetScrollCallback( Window, mousewheel );
}

void RendererGL::setLights()
{  
   const glm::vec4 light_position(50.0f, 150.0f, 50.0f, 1.0f);
   const glm::vec4 ambient_color(0.9f, 0.9f, 0.9f, 1.0f);
   const glm::vec4 diffuse_color(0.9f, 0.9f, 0.9f, 1.0f);
   const glm::vec4 specular_color(0.9f, 0.9f, 0.9f, 1.0f);
   Lights->addLight( light_position, ambient_color, diffuse_color, specular_color );
}

void RendererGL::drawWaveObject()
{
   glUseProgram( WaveShader->getShaderProgram() );
   glUniform1f( WaveShader->getLocation( "WaveFactor" ), WaveObject->getWaveFactor() );
   glUniform2iv( WaveShader->getLocation( "WavePointNumSize" ), 1, &WavePointNumSize[0] );
   
   glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, WaveObject->getWaveBuffer( WaveTargetIndex ) );
   glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, WaveObject->getWaveBuffer( (WaveTargetIndex + 1) % 3 ) );
   glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 2, WaveObject->getWaveBuffer( (WaveTargetIndex + 2) % 3 ) );
   glDispatchCompute( getGroupSize( WavePointNumSize.x ), getGroupSize( WavePointNumSize.y ), 1 );
   glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );

   glUseProgram( WaveNormalShader->getShaderProgram() );
   glUniform2iv( WaveNormalShader->getLocation( "WavePointNumSize" ), 1, &WavePointNumSize[0] );
   glDispatchCompute( getGroupSize( WavePointNumSize.x ), getGroupSize( WavePointNumSize.y ), 1 );
   glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );

   WaveTargetIndex = (WaveTargetIndex + 1) % 3;

   glUseProgram( ObjectShader->getShaderProgram() );
   ObjectShader->transferBasicTransformationUniforms( glm::mat4(1.0f), MainCamera.get() );
   WaveObject->transferUniformsToShader( ObjectShader.get() );
   Lights->transferUniformsToShader( ObjectShader.get() );
   glUniform1i( ObjectShader->getLocation( "LightIndex" ), ActiveLightIndex );
   glUniform1i( ObjectShader->getLocation( "UseTexture" ), 1 );
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
   WaveObject->setWaveObject( WavePointNumSize, WaveGridSize );
   WaveShader->setWaveUniformLocations();
   WaveNormalShader->setWaveNormalUniformLocations();
   ObjectShader->setSceneUniformLocations( Lights->getTotalLightNum() );

   while (!glfwWindowShouldClose( Window )) {
      render();
      
      glfwPollEvents();
      glfwSwapBuffers( Window );
   }
   glfwDestroyWindow( Window );
}