/*
 * Author: Emoy Kim
 * E-mail: emoy.kim_AT_gmail.com
 * 
 * This code is a free software; it can be freely used, changed and redistributed.
 * If you use any version of the code, please reference the code.
 * 
 */

#pragma once

#include "base.h"
#include "light.h"
#include "object.h"

class RendererGL
{
public:
   RendererGL(const RendererGL&) = delete;
   RendererGL(const RendererGL&&) = delete;
   RendererGL& operator=(const RendererGL&) = delete;
   RendererGL& operator=(const RendererGL&&) = delete;


   RendererGL();
   ~RendererGL() = default;

   void play();

private:
   inline static glm::ivec2 ClickedPoint;
   inline static std::unique_ptr<LightGL> Lights;
   inline static std::unique_ptr<CameraGL> MainCamera;

   GLFWwindow* Window;
   int FrameWidth;
   int FrameHeight;
   uint WaveTargetIndex;
   float WaveFactor;
   glm::ivec2 WavePointNumSize;
   glm::ivec2 WaveGridSize;
   std::unique_ptr<ShaderGL> ObjectShader;
   std::unique_ptr<ObjectGL> WaveObject;
 
   void registerCallbacks() const;
   void initialize();

   static void printOpenGLInformation();

   static void cleanup(GLFWwindow* window);
   static void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
   static void cursor(GLFWwindow* window, double xpos, double ypos);
   static void mouse(GLFWwindow* window, int button, int action, int mods);
   static void mousewheel(GLFWwindow* window, double xoffset, double yoffset);
   static void reshape(GLFWwindow* window, int width, int height);

   static void setLights();
   void setWaveObject();
   void drawWaveObject();
   void render();
};