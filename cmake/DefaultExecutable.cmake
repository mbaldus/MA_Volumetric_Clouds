cmake_minimum_required(VERSION 2.8)
if(${CMAKE_MAJOR_VERSION} GREATER 2)
    cmake_policy(SET CMP0038 OLD)
endif()

get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" ProjectId ${ProjectId})
project(${ProjectId})

include_directories(
        ${GLEW_INCLUDE_PATH}
        ${GLFW3_INCLUDE_PATH}
        ${GLM_INCLUDE_PATH}
        ${OpenGL3_INCLUDE_PATH}
        ${EXTERNAL_LIBRARY_PATHS}
        ${LIBRARIES_PATH}
)

file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADER *.h)

add_definitions(-DRESOURCES_PATH="${RESOURCES_PATH}")
add_definitions(-DSHADERS_PATH="${SHADERS_PATH}")
add_definitions(-DTEXTURES_PATH="${TEXTURES_PATH}")
add_definitions(-DGLFW_INCLUDE_GLCOREARB)
add_definitions(-DGLEW_STATIC)

add_executable(${ProjectId} ${SOURCES} ${HEADER})

target_link_libraries(
        ${ProjectId}
        ${ALL_LIBRARIES}
        ${GLEW_LIBRARIES}
        ${GLFW3_LIBRARIES}
        ${OpenGL3_LIBRARIES}
        ${GLM_LIBRARIES}
)