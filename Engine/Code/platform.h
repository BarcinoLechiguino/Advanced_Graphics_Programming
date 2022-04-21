#ifndef __PLATFORM_H__
#define __PLATFORM_H__

// platform.h: 
// Exposes the necessary functions for the Engine to communicate with the Platform layer.

#include "base_types.h"
#include "math_types.h"

struct GLFWwindow;
struct App;

namespace Platform
{
    int  Init();
    void Update(App* app);
    
    void OnGlfwError                (int errorCode, const char* errorMessage);
    
    void OnGlfwMouseMoveEvent       (GLFWwindow* window, double xpos, double ypos);
    void OnGlfwMouseEvent           (GLFWwindow* window, int button, int event, int modifiers);
    void OnGlfwScrollEvent          (GLFWwindow* window, double xoffset, double yoffset);
    void OnGlfwKeyboardEvent        (GLFWwindow* window, int key, int scancode, int action, int mods);
    void OnGlfwCharEvent            (GLFWwindow* window, unsigned int character);
    
    void OnGlfwResizeFramebuffer    (GLFWwindow* window, int width, int height);
    void OnGlfwCloseWindow          (GLFWwindow* window);
}

#endif // !__PLATFORM_H__