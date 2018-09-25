/**
 * @file   		GLTools.h
 * @author Arend Buchacher (abuchacher@uni-koblenz.de)
 *
 * @brief  		A set of useful OpenGL and GLFW related methods.
 *
 * The GLTools methods provide core OpenGL and GLFW functionality methods.
 * There are methods for GLFW Window generation, a generic rendering method,
 * GLFW input callback method binding, glfw window information methods and
 * methods for simple OpenGL error debugging.
 */
#ifndef CVARK_RENDERING_GLTOOLS_H
#define CVARK_RENDERING_GLTOOLS_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
//#include <UI/InputHandling/InputCallbackHandler.h>
//#include "Core/basics.h"
#include <functional>
#include <glm/glm.hpp>
#include <stdio.h>
#include <iostream>
#include <vector>
//#include "Core/Logger.h"



    namespace GLTools
    {
/**
 * @brief Creates a window using GLFW library
 * @details Creates a window registered with the window manager of the operating
 *          system, and also creates an OpenGL context mapped to it.
 *          Also, registerDefaultGLFWCallbacks is called.
 *
 * @param width	 Width of the window
 * @param height Height of the window
 * @param posX 	 Position of the window on the x axis
 * @param posY 	 Position of the window on the y axis
 * @return 		 Pointer to the window instance
 */
        GLFWwindow* generateWindow(int width = 1280, int height = 720,  int posX = 100, int posY = 100, std::string name ="OpenGL Window");

/**
 * @brief Returns whether the window received a close callback
 * @details calls glfwWindowShouldClose
 *
 * @param window to be checked
 * @return true if window should close, else: false
 */
        bool shouldClose(GLFWwindow* window);


/**
 * @brief Swap buffers of window and poll events.
 * @details Calls glfwSwapBuffers and glfwPollEvents
 *
 * @param window to be updated
 */
        void swapBuffers(GLFWwindow* window);

/**
 * @brief destroy window and terminate GLFW.
 * @details Calls glfwSwapBuffers and glfwPollEvents
 *
 * @param window to be destroyed
 */
        void destroyWindow(GLFWwindow* window);

/**
 * @brief Default Render-Loop
 * @details Runs the default render loop function. Waits for closeing window
 *          events from GLFW. Also calculates a delta time variable for measure
 *          time between render loop calls
 *
 * @param window Thw window to render to
 * @param loop   Function pointer of an extern function called from within the
 *               render loop.
 */
		void render(GLFWwindow* window, std::function<void(double)> loop);

/**
 * @brief Registers a keyboard button callback function
 * @details Pass a function pointer to your keyboard button callback function
 *          and it will be registered with the given window. The keyboard
 *          function gets called, when a button of the keyboard gets pressed.
 *
 * @param window The window to register the callback to
 * @param func The function pointer of the callback function to register
 */
        void setKeyCallback(GLFWwindow* window, std::function<void (int, int, int, int)> func);

/**
 * @brief Registers a mouse button callback function
 * @details Pass a function pointer to your mouse button callback function
 *          and it will be registered with the given window. The mouse button
 *          function gets called, when a button of the mouse gets pressed.
 *
 * @param window The window to register the callback to
 * @param func The function pointer of the callback function to register
 */
        void setMouseButtonCallback(GLFWwindow* window, std::function<void (int, int, int)> func);

/**
 * @brief Registers a character callback function
 * @details Pass a function pointer to your character callback function and it
 *          will be registered with the given window. The character function
 *          gets called, when a single unicode character is input.
 *
 * @param window The window to register the callback to
 * @param func The function pointer of the callback function to register
 */
        void setCharCallback(GLFWwindow* window, std::function<void (unsigned int)> func);

/**
 * @brief Registers a cursor position callback function
 * @details Pass a function pointer to your cursor position callback function
 *          and it will be registered with the given window. The cursor position
 *          function gets called, when the cursor gets moved on an active
 *          window.
 *
 * @param window The window to register the callback to
 * @param func The function pointer of the callback function to register
 */
        void setCursorPosCallback(GLFWwindow* window, std::function<void (double, double)> func);

/**
 * @brief Registers a scroll callback function
 * @details Pass a function pointer to your scroll callback function and it will
 *          be registered with the given window. The scroll function gets called
 *          when the mouse scroll wheel is used on an active window.
 *
 * @param window The window to register the callback to
 * @param func The function pointer of the callback function to register
 */
        void setScrollCallback(GLFWwindow* window, std::function<void (double, double)> func);

/**
 * @brief Registers a cursor enter callback function
 * @details Pass a function pointer to your cursor enter callback function and
 *          it will be registered with the given window. The cursor enter
 *          function gets called when entering the window area with the mouse.
 *
 * @param window The window to register the callback to
 * @param func The function pointer of the callback function to register
 */
        void setCursorEnterCallback(GLFWwindow* window, std::function<void (int)> func);

/**
 * @brief Returns the windows width
 *
 * @param window Window to get the width of
 * @return The width of the given window
 */
        int getWidth(GLFWwindow* window);

/**
 * @brief Returns the windows height
 *
 * @param window Window to get the height of
 * @return The height of the given window
 */
        int getHeight(GLFWwindow* window);

/**
 * @brief Returns the windows aspect ratio
 *
 * @param window Window to get the ratio of
 * @return The ratio of the given window
 */
        glm::vec2 getResolution(GLFWwindow* window);

/**
 * @brief Returns the windows aspect ratio
 *
 * @param window Window to get the ratio of
 * @return The ratio of the given window
 */
        float getRatio(GLFWwindow* window);

/**
 * @brief Queries and prints the current error state of OpenGL.
 * @details This method may be used to perform some simple OpenGL debugging.
 * It calls glGetError and prints the corresponding errorname to the console.
 * Wrap this method around GL-commands to find out where an erroneous state is produced.
 * @param printIfNoError (optional) set to true if GL_NO_ERROR state shall be printed aswell.
 * @return current error state.
 */
        GLenum checkGLError(bool printIfNoError = false);

/**
 * @brief Queries and prints the current error state of OpenGL.
 * @details This method may be used to perform some simple OpenGL debugging.
 * It calls glCheckFramebufferStatus(GL_FRAMEBUFFER) and prints the corresponding error to the console.
 * Wrap this method around GL-commands to find out where an erroneous state is produced.
 * @param printIfNoError (optional) set to true if GL_NO_ERROR state shall be printed aswell.
 * @return current error state.
 */
        GLenum checkFramebuffer(bool printIfNoError = false);

/**
 * @brief Converts the provided OpenGL error state to a human readable string.
 * @details This method may be used to retrieve the corresponding errorname of an error state.
 * Use this method with checkGLError or glGetError().
 * @param error the OpenGL error state to which the name shall be retrieved.
 * @return error state as a string, or an empty string, if error is no valid OpenGL error state.
 */
        std::string decodeGLError(GLenum error);

/**
 * @brief binds the static callback functions defined in GLTools.cpp as GLFWCallback functions.
 * @details this method uses the regular callback setters to bind the following callbacks:
 * 		keyCallback
 * 		mouseButtonCallback
 * 		cursorPosCallback
 * 		scrollCallback
 * 	Functions, that are supposed to be called with the respective event, may be added via the
 * 	add callback functions.
 * 	Note that this is designed for use with only one GLFW-Window, since callbacks would be
 * 	called twice when calling this method with another window.
 * @param window
 */
        void registerDefaultGLFWCallbacks(GLFWwindow* window);

/**
 * @brief add a function to the set of functions, that will be called when a key callback occurs.
 * @details registerDefaultGLFWCallbacks must have been called to ensure the default callbacks are bound.
 * 	the provided function should receive the GLFWkeyfun arguments:
 * 		(int key, int scan, int action, int mods)
 * @param cbFunction to be called when a glfwKeyCallback occurs
 */
        void addKeyCallbackFunction( std::function<void(int, int, int, int)> cbFunction );
/**
 * @brief add a function to the set of functions, that will be called when a mouse button callback occurs.
 * @details registerDefaultGLFWCallbacks must have been called to ensure the default callbacks are bound.
 * 	the provided function should receive the  GLFWmousebuttonfun arguments:
 * 		(int button, int action, int mods)
 * @param cbFunction to be called when a glfwKeyCallback occurs
 */
        void addMouseButtonCallbackFunction( std::function< void(int, int, int)> cbFunction );
/**
 * @brief add a function to the set of functions, that will be called when a mouse button callback occurs.
 * @details registerDefaultGLFWCallbacks must have been called to ensure the default callbacks are bound.
 * 	the provided function should receive the GLFWcursorposfun arguments:
 * 		(double x, double y)
 * @param cbFunction to be called when a glfwKeyCallback occurs
 */
        void addCursorPosCallbackFunction( std::function<void(double, double)> cbFunction);

/**
 * @brief add a function to the set of functions, that will be called when a mouse button callback occurs.
 * @details registerDefaultGLFWCallbacks must have been called to ensure the default callbacks are bound.
 * 	the provided function should receive the GLFWscrollfun arguments:
 * 		(double xoffset, yoffset)
 * @param cbFunction to be called when a glfwScrollCallback occurs
 */
        void addScrollCallbackFunction( std::function<void(double, double)> cbFunction );

/**
 * @brief get the texture dimensions for the given texture handle
 * @details throws exception if textureHandle is not a valid texture
 * @param OpenGL texture handle
 */
        glm::vec2 getTextureSize(GLuint textureHandle);


    } // namespace GLTools


#endif // CVARK_RENDERING_GLTOOLS_H
