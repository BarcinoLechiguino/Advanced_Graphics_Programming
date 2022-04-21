#ifndef __PLATFORM_H__
#define __PLATFORM_H__

//
// platform.h : This file contains basic platform types and tools. Also, it exposes
// the necessary functions for the Engine to communicate with the Platform layer.
//

//#pragma once
//#pragma warning(disable : 4267)                                     // conversion from X to Y, possible loss of data

#include <math.h>
#include <vector>
#include <string>

#include "base_types.h"
#include "math_types.h"

namespace OpenGL
{

}

class Platform
{
public:
    int     InitPlat();

private:
    //vars
};

#endif // !__PLATFORM_H__