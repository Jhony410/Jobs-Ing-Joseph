#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// Función para cargar shaders
unsigned int crearProgramaShader(const char* rutaVertex, const char* rutaFragment) {
    std::ifstream vShaderFile(rutaVertex), fShaderFile(rutaFragment);
    std::stringstream vShaderStream, fShaderStream;
    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();
    std::string vCode = vShaderStream.str(), fCode = fShaderStream.str();
    const char* vShaderCode = vCode.c_str();
    const char* fShaderCode = fCode.c_str();

    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);

    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
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

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL House", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    unsigned int shaderProgram = crearProgramaShader("src/vertex.glsl", "src/fragment.glsl");

    // --- GEOMETRÍA DE LA CASA: 5 Vértices ---
    float vertices[] = {
        // Posiciones (X, Y, Z)    // Colores (R, G, B)
         0.5f,  0.0f, 0.0f,    1.0f, 0.0f, 0.0f,  // 0: Arriba-Der Pared (Rojo)
         0.5f, -0.6f, 0.0f,    0.0f, 1.0f, 0.0f,  // 1: Abajo-Der Pared (Verde)
        -0.5f, -0.6f, 0.0f,    0.0f, 0.0f, 1.0f,  // 2: Abajo-Izq Pared (Azul)
        -0.5f,  0.0f, 0.0f,    1.0f, 1.0f, 0.0f,  // 3: Arriba-Izq Pared (Amarillo)
         0.0f,  0.6f, 0.0f,    1.0f, 0.0f, 1.0f   // 4: Punta del Techo (Magenta)
    };
    // --- ÍNDICES: 3 Triángulos ---
    unsigned int indices[] = {  
        0, 1, 3,   // Primer triángulo de la pared
        1, 2, 3,   // Segundo triángulo de la pared
        3, 0, 4    // Triángulo del techo
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Atributos: Posición (0) y Color (1)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        // Dibujamos 9 índices (3 triángulos)
        glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();
    return 0;
}