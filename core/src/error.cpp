#include "core/error.hpp"
#include <iostream>
#include <GL/glew.h>

bool glCheckError()
{
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        std::cout << "GL Error: " << error << std::endl;
    }
    return true;
}

void glClearError()
{
    while (glGetError() != GL_NO_ERROR);
}
