#ifndef SPH_FLUID_SOLVER_SHADER_UTILITIES_H
#define SPH_FLUID_SOLVER_SHADER_UTILITIES_H
#include <string>
#include <glad/glad.h>

float inline quadVertices[] = {
    -1.0f, -1.0f,
    1.0f, -1.0f,
    1.0f, 1.0f,
    -1.0f, 1.0f
};

unsigned inline int quadIndices[] = {
    0, 1, 2,
    2, 3, 0
};

std::string loadFile(const std::string& path);
GLuint createShader(GLenum type, const std::string& source);
GLuint createShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource);
GLuint createVAO();

#endif //SPH_FLUID_SOLVER_SHADER_UTILITIES_H
