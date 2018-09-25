if(WIN32)
cmake_minimum_required(VERSION 3.0.2)
else()
cmake_minimum_required(VERSION 2.8)
endif(WIN32)

set(CMAKE_MODULE_PATH  ${CMAKE_SOURCE_DIR}/cmake CACHE PATH "Project specific path. Set manually if it was not found.")
set(EXECUTABLE_PATH ${CMAKE_SOURCE_DIR}/src/executables CACHE PATH "Project specific path. Set manually if it was not found.")
set(LIBRARIES_PATH ${CMAKE_SOURCE_DIR}/src/libraries CACHE PATH "Project specific path. Set manually if it was not found.")
set(TEXTURES_PATH ${CMAKE_SOURCE_DIR}/src/textures CACHE PATH "Project specific path. Set manually if it was not found.")
set(SHADERS_PATH ${CMAKE_SOURCE_DIR}/src/shaders CACHE PATH "Project specific path. Set manually if it was not found.")
set(DEPENDENCIES_ROOT ${CMAKE_SOURCE_DIR}/dependencies CACHE PATH "Project specific path. Set manually if it was not found.")
set(RESOURCES_PATH ${CMAKE_SOURCE_DIR}/resources CACHE PATH "Project specific path. Set manually if it was not found.")
set(MODEL_PATH ${CMAKE_SOURCE_DIR}/models CACHE PATH "Project specific path. Set manually if it was not found.")

add_definitions ( -D_MODEL_PATH="${MODEL_PATH}")

include(${CMAKE_MODULE_PATH}/DefaultProject.cmake)
