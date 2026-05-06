#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// --- FUNCIÓN PARA LEER Y COMPILAR SHADERS AUTOMÁTICAMENTE ---
unsigned int crearProgramaShader(const char* rutaVertex, const char* rutaFragment) {
    std::ifstream vShaderFile(rutaVertex), fShaderFile(rutaFragment);
    std::stringstream vShaderStream, fShaderStream;
    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();
    std::string vertexCode = vShaderStream.str();
    std::string fragmentCode = fShaderStream.str();
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex, fragment;
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);

    unsigned int ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    
    return ID;
}
// -----------------------------------------------------------

int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Plantilla OpenGL - EBO Cuadrado", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    unsigned int shaderProgram = crearProgramaShader("src/vertex.glsl", "src/fragment.glsl");

    // --- GEOMETRÍA: 4 Vértices para un cuadrado ---
    // Posiciones (X, Y, Z)    // Colores (R, G, B)
    float vertices[] = {
         0.5f,  0.5f, 0.0f,    1.0f, 0.0f, 0.0f,  // 0: Arriba a la derecha (Rojo)
         0.5f, -0.5f, 0.0f,    0.0f, 1.0f, 0.0f,  // 1: Abajo a la derecha (Verde)
        -0.5f, -0.5f, 0.0f,    0.0f, 0.0f, 1.0f,  // 2: Abajo a la izquierda (Azul)
        -0.5f,  0.5f, 0.0f,    1.0f, 1.0f, 0.0f   // 3: Arriba a la izquierda (Amarillo)
    };

    // --- ÍNDICES: Cómo conectar los vértices ---
    unsigned int indices[] = {  
        0, 1, 3,   // Primer triángulo (Arriba-Der, Abajo-Der, Arriba-Izq)
        1, 2, 3    // Segundo triángulo (Abajo-Der, Abajo-Izq, Arriba-Izq)
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO); // Generamos el buffer para los índices

    // 1. Vinculamos el VAO primero
    glBindVertexArray(VAO);

    // 2. Copiamos nuestros vértices al buffer (VBO)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 3. Copiamos nuestros índices al buffer (EBO)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 4. Configuramos los atributos (Posición y Color)
    // Posición (Location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Color (Location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Bucle principal
    while (!glfwWindowShouldClose(window)) {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        
        // ¡CAMBIO CLAVE! Usamos glDrawElements en lugar de glDrawArrays
        // Parámetros: Tipo de primitiva, cantidad de índices (6), tipo de dato de los índices, offset (0)
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}