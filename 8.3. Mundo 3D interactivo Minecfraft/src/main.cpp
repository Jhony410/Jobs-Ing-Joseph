#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// --- GLOBALES DE LA CÁMARA ORBITAL ---
float radius = 5.0f;
float camYaw = 45.0f;
float camPitch = 30.0f;
float cubeRotation = 0.0f;

// --- NUEVO: Variables de tiempo (DeltaTime) ---
float deltaTime = 0.0f;	// Tiempo entre el frame actual y el anterior
float lastFrame = 0.0f; // Tiempo del último frame

// Variables del ratón
bool firstMouse = true;
float lastX = 800.0f / 2.0;
float lastY = 600.0f / 2.0;

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.3f;
    camYaw += xoffset * sensitivity;
    camPitch += yoffset * sensitivity;

    if (camPitch > 89.0f) camPitch = 89.0f;
    if (camPitch < -89.0f) camPitch = -89.0f;
}

// --- NUEVO: Procesamiento del teclado usando deltaTime ---
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Definimos velocidades por segundo
    float zoomSpeed = 5.0f * deltaTime;       // 5 unidades de distancia por segundo
    float rotationSpeed = 90.0f * deltaTime;  // 90 grados por segundo

    // Zoom
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) radius -= zoomSpeed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) radius += zoomSpeed;
    if (radius < 1.5f) radius = 1.5f; 
    if (radius > 15.0f) radius = 15.0f;

    // Rotar el modelo
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cubeRotation -= rotationSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cubeRotation += rotationSpeed;
}

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
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Persona 3D - Low Poly", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    glEnable(GL_DEPTH_TEST);

    unsigned int shaderProgram = crearProgramaShader("src/vertex.glsl", "src/fragment.glsl");

    // Cubo unitario centrado en origen — se reusa para cada parte del cuerpo
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int textura;
    glGenTextures(1, &textura);
    glBindTexture(GL_TEXTURE_2D, textura);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(true); 
    int width, height, nrChannels;
    unsigned char *data = stbi_load("fondo-pared-ladrillo.png", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Error al cargar la textura" << std::endl;
    }
    stbi_image_free(data);

    while (!glfwWindowShouldClose(window)) {
        // --- NUEVO: Cálculo del DeltaTime en cada frame ---
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        glBindTexture(GL_TEXTURE_2D, textura);
        glUseProgram(shaderProgram);

        // Rotación global de toda la figura
        glm::mat4 baseRot = glm::rotate(glm::mat4(1.0f),
                                        glm::radians(cubeRotation),
                                        glm::vec3(0.0f, 1.0f, 0.0f));

        float camX = sin(glm::radians(camYaw)) * cos(glm::radians(camPitch)) * radius;
        float camY = sin(glm::radians(camPitch)) * radius;
        float camZ = cos(glm::radians(camYaw)) * cos(glm::radians(camPitch)) * radius;

        glm::mat4 view = glm::lookAt(
            glm::vec3(camX, camY, camZ),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc  = glGetUniformLocation(shaderProgram, "view");
        unsigned int projLoc  = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Partes del cuerpo: {posición relativa al centro de la figura, escala}
        // Y=0 es la cintura. Cabeza arriba, piernas abajo.
        struct BodyPart { glm::vec3 pos; glm::vec3 scale; };
        BodyPart partes[] = {
            { glm::vec3( 0.00f,  0.80f,  0.00f), glm::vec3(0.40f, 0.40f, 0.35f) }, // cabeza
            { glm::vec3( 0.00f,  0.10f,  0.00f), glm::vec3(0.60f, 0.70f, 0.30f) }, // torso
            { glm::vec3(-0.42f,  0.05f,  0.00f), glm::vec3(0.20f, 0.65f, 0.20f) }, // brazo izquierdo
            { glm::vec3( 0.42f,  0.05f,  0.00f), glm::vec3(0.20f, 0.65f, 0.20f) }, // brazo derecho
            { glm::vec3(-0.16f, -0.65f,  0.00f), glm::vec3(0.25f, 0.72f, 0.25f) }, // pierna izquierda
            { glm::vec3( 0.16f, -0.65f,  0.00f), glm::vec3(0.25f, 0.72f, 0.25f) }, // pierna derecha
        };

        glBindVertexArray(VAO);
        for (auto& parte : partes) {
            glm::mat4 model = glm::translate(baseRot, parte.pos);
            model = glm::scale(model, parte.scale);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    
    return 0;
}