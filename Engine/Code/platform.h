#ifndef __PLATFORM_H__
#define __PLATFORM_H__

//
// platform.h : This file contains basic platform types and tools. Also, it exposes
// the necessary functions for the Engine to communicate with the Platform layer.
//

//#pragma once
//#pragma warning(disable : 4267)                                     // conversion from X to Y, possible loss of data

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <vector>
#include <string>

#include "base_types.h"
#include "math_types.h"

enum MouseButton
{
    LEFT,
    RIGHT,
    MOUSE_BUTTON_COUNT
};

enum Key
{
    K_SPACE,

    K_0, K_1, K_2, K_3, K_4, K_5, K_6, K_7, K_8, K_9,

    K_A, K_B, K_C, K_D, K_E, K_F, K_G, K_H, K_I, K_J, K_K, K_L, K_M,
    K_N, K_O, K_P, K_Q, K_R, K_S, K_T, K_U, K_V, K_W, K_X, K_Y, K_Z,

    K_ENTER, K_ESCAPE,

    KEY_COUNT
};

enum ButtonState
{
    BUTTON_IDLE,
    BUTTON_PRESS,
    BUTTON_PRESSED,
    BUTTON_RELEASE
};

struct Input
{
    glm::vec2   mousePos;
    glm::vec2   mouseDelta;
    ButtonState mouseButtons[MOUSE_BUTTON_COUNT];
    ButtonState keys[KEY_COUNT];
};

struct String
{
    char* str;
    u32   len;
};

class Platform
{

};

int     Init();

String  MakeString                  (const char *cstr);
String  MakePath                    (String dir, String filename);
String  GetDirectoryPart            (String path);

String  ReadTextFile                (const char *filepath);         // Reads a whole file and returns a string with its contents. The returned string is temporary and should be copied if it needs to persist for several frames.
u64     GetFileLastWriteTimestamp   (const char *filepath);         // It retrieves a timestamp indicating the last time the file was modified. Can be useful in order to check for file modifications to implement hot reloads.

#endif // !__PLATFORM_H__