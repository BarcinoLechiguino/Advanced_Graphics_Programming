#include "engine.h"

int main()
{
    App* app = new App();
    
    int val = app->platform.InitPlat();

    delete app;

    return val;
}