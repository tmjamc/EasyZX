#include <windows.h>
#include <iostream>
#include <format>

#include "shader.h"
#include "glad/gl.h"
#include "win_app.h"

namespace shader
{
    unsigned int shaderId = 0;

    static const char* getShaderSourceFromResource(const int resourceId)
    {
        HMODULE hModule = GetModuleHandle(nullptr);

        HRSRC hRes = FindResource(hModule, MAKEINTRESOURCE(resourceId), TEXT("SHADER"));
        if (hRes == nullptr)
        {
            OutputDebugStringA("Failed to find resource\n");
            return nullptr;
        }

        HGLOBAL hGlob = LoadResource(hModule, hRes);
        if (hGlob == nullptr)
        {
            OutputDebugStringA("Failed to load resource\n");
            return nullptr;
        }

        return static_cast<const char*>(LockResource(hGlob));
    }

    static void checkCompileErrors(unsigned int shader, const char* type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                win_app::error(std::format("ERROR::SHADER_COMPILATION_ERROR of type: {}\n{}\n -- --------------------------------------------------- -- ", type, infoLog).c_str());
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                win_app::error(std::format("ERROR::PROGRAM_LINKING_ERROR of type: {}\n{}\n -- --------------------------------------------------- -- ", type, infoLog).c_str());
            }
        }
    }

    void compile(const int vertexResourceId, const int fragmentResourceId)
    {
        // compile shaders
        unsigned int vertex, fragment;

        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        const char* vertexSource = getShaderSourceFromResource(vertexResourceId);
        glShaderSource(vertex, 1, &vertexSource, nullptr);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");

        // fragment shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        const char* fragmentSource = getShaderSourceFromResource(fragmentResourceId);
        glShaderSource(fragment, 1, &fragmentSource, nullptr);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");

        // shader Program
        shaderId = glCreateProgram();
        glAttachShader(shaderId, vertex);
        glAttachShader(shaderId, fragment);
        glLinkProgram(shaderId);
        checkCompileErrors(shaderId, "PROGRAM");

        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    void cleanUp()
    {
        if (shaderId == 0)
        {
            return;
        }

        glDeleteProgram(shaderId);
    }

    void setVec2(const char* name, float x, float y)
    {
        glUniform2f(glGetUniformLocation(shaderId, name), x, y);
    }
}