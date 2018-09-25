string(REPLACE "/" ";" p2list "${CMAKE_SOURCE_DIR}")
string(REPLACE "\\" ";" p2list "${p2list}")
list(REVERSE p2list)
list(GET p2list 0 first)
list(GET p2list 1 ProjectId)
string(REPLACE " " "_" ProjectId ${ProjectId})

if(WIN32)
get_filename_component(ProjectId "${CMAKE_SOURCE_DIR}" NAME)
endif(WIN32)

project(${ProjectId})

include(${CMAKE_MODULE_PATH}/macros.cmake)

set(CMAKE_CONFIGURATION_TYPES Debug; Release)

if(WIN32)
file(GLOB_RECURSE USED_DLLS ${DEPENDENCIES_ROOT}/*.dll)
else(WIN32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif(WIN32)

find_package(OpenGL3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(GLM REQUIRED)


set(USE_OPENGL_CORE_PROFILE_330 1 CACHE BOOL "Use OpenGL Core Profile")
if(${USE_OPENGL_CORE_PROFILE_330})
    add_definitions(-DCVARK_USE_CORE_PROFILE_330)
endif()

if("${CMAKE_SYSTEM}" MATCHES "Linux")
    find_package(X11)
    set(ALL_LIBRARIES ${ALL_LIBRARIES} ${X11_LIBRARIES} Xrandr Xxf86vm Xi pthread)
endif()

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
GENERATE_SUBDIRS(ALL_LIBRARIES ${LIBRARIES_PATH} ${PROJECT_BINARY_DIR}/libraries)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
GENERATE_SUBDIRS(ALL_EXECUTABLES ${EXECUTABLE_PATH} ${PROJECT_BINARY_DIR}/executables)

if(EXISTS ${SHADERS_PATH})
    add_subdirectory(${SHADERS_PATH})
endif()