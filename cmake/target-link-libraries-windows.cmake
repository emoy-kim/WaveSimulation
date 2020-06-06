target_link_libraries(WaveSimulation glad glfw3dll)

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
   target_link_libraries(WaveSimulation FreeImaged)
else()
   target_link_libraries(WaveSimulation FreeImage)
endif()