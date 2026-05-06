#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Librería para cargar imágenes
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Librerías matemáticas GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// Función para compilar shaders
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
    // Configuración básica
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL - Texturas y Movimiento Fluido", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    unsigned int shaderProgram = crearProgramaShader("src/vertex.glsl", "src/fragment.glsl");

    // --- GEOMETRÍA ---
    float vertices[] = {
        // Posiciones (X,Y,Z)   // Colores (R,G,B)   // Texturas (U,V)
         0.5f,  0.5f, 0.0f,     1.0f, 0.0f, 0.0f,    1.0f, 1.0f, // Arriba-Der
         0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f,    1.0f, 0.0f, // Abajo-Der
        -0.5f, -0.5f, 0.0f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f, // Abajo-Izq
        -0.5f,  0.5f, 0.0f,     1.0f, 1.0f, 0.0f,    0.0f, 1.0f  // Arriba-Izq 
    };

    unsigned int indices[] = {  
        0, 1, 3, 
        1, 2, 3  
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

    // Atributos: Posición, Color y Textura (Stride = 8)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // --- TEXTURA ---
    unsigned int textura;
    glGenTextures(1, &textura);
    glBindTexture(GL_TEXTURE_2D, textura);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(true); 
    int width, height, nrChannels;
    unsigned char *data = stbi_load("logo.png", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Error al cargar la textura" << std::endl;
    }
    stbi_image_free(data);

    // --- ESTADO Y TIEMPO ---
    float posX = 0.0f;
    float posY = 0.0f;
    
    float deltaTime = 0.0f; 
    float lastFrame = 0.0f; 

    // --- BUCLE INFINITO ---
    while (!glfwWindowShouldClose(window)) {
        // 1. Calcular el Delta Time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 2. Interacción con teclado
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        
        // La velocidad ahora se multiplica por el deltaTime para ser consistente
        float velocidad = 1.0f * deltaTime; 
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) posX += velocidad;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  posX -= velocidad;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    posY += velocidad;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  posY -= velocidad;

        // 3. Limpiar pantalla
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 4. Preparar textura y shader
        glBindTexture(GL_TEXTURE_2D, textura);
        glUseProgram(shaderProgram);

        // 5. Crear matriz y enviarla a la GPU
        glm::mat4 trans = glm::mat4(1.0f); 
        trans = glm::translate(trans, glm::vec3(posX, posY, 0.0f)); 

        unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transformacion");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

        // 6. Dibujar
        glBindVertexArray(VAO);
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