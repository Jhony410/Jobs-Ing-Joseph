#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

// ============================== VISTA ==============================
// CLASE Shader: encapsula un programa de GPU (vertex + fragment).
// Se encarga de leer los archivos GLSL, compilarlos, enlazarlos
// y enviar los uniforms desde C++ hacia los shaders.
class Shader {
public:
    unsigned int ID = 0;   // identificador del programa en OpenGL

    // Lee, compila y enlaza los dos shaders desde archivo.
    void load(const char* vPath, const char* fPath) {
        std::string vCode = readFile(vPath), fCode = readFile(fPath);
        unsigned int v = compile(GL_VERTEX_SHADER,   vCode.c_str(), vPath);
        unsigned int f = compile(GL_FRAGMENT_SHADER, fCode.c_str(), fPath);
        ID = glCreateProgram();
        glAttachShader(ID, v); glAttachShader(ID, f);
        glLinkProgram(ID);
        glDeleteShader(v); glDeleteShader(f);
    }

    // Activa este programa para los siguientes dibujados.
    void use() const { glUseProgram(ID); }

    // Envia una matriz 4x4 al shader.
    void setMat4(const char* n, const glm::mat4& m) const {
        glUniformMatrix4fv(loc(n), 1, GL_FALSE, glm::value_ptr(m)); }

    // Envia un vector de 3 componentes al shader.
    void setVec3(const char* n, const glm::vec3& v) const {
        glUniform3fv(loc(n), 1, glm::value_ptr(v)); }

    // Envia un arreglo de vectores vec3 al shader.
    void setVec3Array(const char* n, const glm::vec3* v, int count) const {
        glUniform3fv(loc(n), count, glm::value_ptr(v[0])); }

    // Envia un numero flotante al shader.
    void setFloat(const char* n, float f) const { glUniform1f(loc(n), f); }

    // Envia un entero (por ejemplo, la unidad de textura) al shader.
    void setInt(const char* n, int i) const { glUniform1i(loc(n), i); }

private:
    // Busca la ubicacion de un uniform por nombre.
    int loc(const char* n) const { return glGetUniformLocation(ID, n); }

    // Lee el contenido completo de un archivo de texto.
    static std::string readFile(const char* path) {
        std::ifstream f(path); std::stringstream ss; ss << f.rdbuf();
        return ss.str();
    }

    // Compila un shader y reporta errores por consola si los hay.
    static unsigned int compile(GLenum type, const char* src, const char* name) {
        unsigned int s = glCreateShader(type);
        glShaderSource(s, 1, &src, NULL); glCompileShader(s);
        int ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) { char log[1024]; glGetShaderInfoLog(s, 1024, NULL, log);
                   std::cout << "[Shader] " << name << ":\n" << log << std::endl; }
        return s;
    }
};
