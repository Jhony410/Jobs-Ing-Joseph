#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

struct MaterialData {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
};

float radius = 15.0f;
float camYaw = 45.0f;
float camPitch = 15.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool firstMouse = true;
float lastX = 400, lastY = 300;

MaterialData materials[] = {
    // 0 y 5. GOMA MATE NEGRA 
    {glm::vec3(0.02f, 0.02f, 0.02f), glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(0.0f, 0.0f, 0.0f), 0.01f},
    
    // 1 y 6. CROMO / ESPEJO 
    {glm::vec3(0.25f, 0.25f, 0.25f), glm::vec3(0.4f, 0.4f, 0.4f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f},
    
    // 2 y 7. PLÁSTICO AZUL 
    {glm::vec3(0.0f, 0.1f, 0.2f), glm::vec3(0.0f, 0.4f, 0.8f), glm::vec3(0.5f, 0.5f, 0.5f), 0.25f},
    
    // 3 y 8. ORO PURO METÁLICO 
    {glm::vec3(0.2f, 0.15f, 0.0f), glm::vec3(0.6f, 0.5f, 0.1f), glm::vec3(1.0f, 0.8f, 0.2f), 0.6f},
    
    // 4 y 9. ARCILLA ROJA 
    {glm::vec3(0.2f, 0.05f, 0.0f), glm::vec3(0.6f, 0.2f, 0.1f), glm::vec3(0.05f, 0.05f, 0.05f), 0.05f}
};

void crearEsfera(std::vector<float>& vertices, int rings, int sectors) {
    float R = 1.0f / (float)(rings - 1); float S = 1.0f / (float)(sectors - 1);
    const float PI = 3.14159265359f;
    std::vector<glm::vec3> positions; std::vector<glm::vec3> normals;

    for(int r = 0; r < rings; r++) {
        for(int s = 0; s < sectors; s++) {
            float y = sin(-PI/2.0f + PI * r * R);
            float x = cos(2*PI * s * S) * sin(PI * r * R);
            float z = sin(2*PI * s * S) * sin(PI * r * R);
            positions.push_back(glm::vec3(x, y, z));
            normals.push_back(glm::normalize(glm::vec3(x, y, z)));
        }
    }

    for(int r = 0; r < rings - 1; r++) {
        for(int s = 0; s < sectors - 1; s++) {
            int i0 = r * sectors + s; int i1 = r * sectors + (s + 1);
            int i2 = (r + 1) * sectors + (s + 1); int i3 = (r + 1) * sectors + s;

            vertices.push_back(positions[i0].x); vertices.push_back(positions[i0].y); vertices.push_back(positions[i0].z);
            vertices.push_back(normals[i0].x); vertices.push_back(normals[i0].y); vertices.push_back(normals[i0].z);
            vertices.push_back(positions[i1].x); vertices.push_back(positions[i1].y); vertices.push_back(positions[i1].z);
            vertices.push_back(normals[i1].x); vertices.push_back(normals[i1].y); vertices.push_back(normals[i1].z);
            vertices.push_back(positions[i2].x); vertices.push_back(positions[i2].y); vertices.push_back(positions[i2].z);
            vertices.push_back(normals[i2].x); vertices.push_back(normals[i2].y); vertices.push_back(normals[i2].z);

            vertices.push_back(positions[i0].x); vertices.push_back(positions[i0].y); vertices.push_back(positions[i0].z);
            vertices.push_back(normals[i0].x); vertices.push_back(normals[i0].y); vertices.push_back(normals[i0].z);
            vertices.push_back(positions[i2].x); vertices.push_back(positions[i2].y); vertices.push_back(positions[i2].z);
            vertices.push_back(normals[i2].x); vertices.push_back(normals[i2].y); vertices.push_back(normals[i2].z);
            vertices.push_back(positions[i3].x); vertices.push_back(positions[i3].y); vertices.push_back(positions[i3].z);
            vertices.push_back(normals[i3].x); vertices.push_back(normals[i3].y); vertices.push_back(normals[i3].z);
        }
    }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn); float ypos = static_cast<float>(yposIn);
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX; float yoffset = lastY - ypos; 
    lastX = xpos; lastY = ypos;
    camYaw += xoffset * 0.1f; camPitch += yoffset * 0.1f;
    if (camPitch > 89.0f) camPitch = 89.0f; if (camPitch < -89.0f) camPitch = -89.0f;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    float zoomSpeed = 8.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) radius -= zoomSpeed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) radius += zoomSpeed;
}

unsigned int crearProgramaShader(const char* rutaVertex, const char* rutaFragment) {
    std::ifstream vShaderFile(rutaVertex), fShaderFile(rutaFragment);
    std::stringstream vShaderStream, fShaderStream;
    vShaderStream << vShaderFile.rdbuf(); fShaderStream << fShaderFile.rdbuf();
    std::string vCode = vShaderStream.str(), fCode = fShaderStream.str();
    const char* vShaderCode = vCode.c_str(); const char* fShaderCode = fCode.c_str();
    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL); glCompileShader(vertex);
    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL); glCompileShader(fragment);
    unsigned int ID = glCreateProgram();
    glAttachShader(ID, vertex); glAttachShader(ID, fragment);
    glLinkProgram(ID); glDeleteShader(vertex); glDeleteShader(fragment);
    return ID;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1000, 800, "Contrastes Extremos", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_DEPTH_TEST);

    unsigned int shaderProgram = crearProgramaShader("src/vertex.glsl", "src/fragment.glsl");

    std::vector<float> sphereVertices;
    crearEsfera(sphereVertices, 60, 60); 
    int numVertices = sphereVertices.size() / 6; 

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO);
    glBindVertexArray(VAO); glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);

    glUseProgram(shaderProgram);

    glm::vec3 posicionesEsferas[10];
    for(int i = 0; i < 10; i++) {
        float angle = glm::radians(36.0f * i);
        posicionesEsferas[i] = glm::vec3(cos(angle) * 8.0f, 0.0f, sin(angle) * 8.0f);
    }

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame; lastFrame = currentFrame;
        processInput(window);
        
        glClearColor(0.02f, 0.02f, 0.02f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        float camX = sin(glm::radians(camYaw)) * cos(glm::radians(camPitch)) * radius;
        float camY = sin(glm::radians(camPitch)) * radius;
        float camZ = cos(glm::radians(camYaw)) * cos(glm::radians(camPitch)) * radius;
        glm::vec3 cameraPos = glm::vec3(camX, camY, camZ);
        
        glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1000.0f/800.0f, 0.1f, 100.0f);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);  
        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);

        float lightX = 4.0f * sin(currentFrame * 2.0f);
        float lightZ = 4.0f * cos(currentFrame * 2.0f);
        float lightY = 3.0f * sin(currentFrame * 3.0f); 
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), lightX, lightY, lightZ);

        glBindVertexArray(VAO);
        
        for (unsigned int i = 0; i < 10; i++) {
            
            int matIndex = i % 5; 
            
            glUniform3fv(glGetUniformLocation(shaderProgram, "material.ambient"), 1, glm::value_ptr(materials[matIndex].ambient));
            glUniform3fv(glGetUniformLocation(shaderProgram, "material.diffuse"), 1, glm::value_ptr(materials[matIndex].diffuse));
            glUniform3fv(glGetUniformLocation(shaderProgram, "material.specular"), 1, glm::value_ptr(materials[matIndex].specular));
            glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), materials[matIndex].shininess);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, posicionesEsferas[i]);
            model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f)); 
            model = glm::translate(model, glm::vec3(0.0f, sin(currentFrame + i) * 0.5f, 0.0f));

            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES, 0, numVertices);
        }

        glfwSwapBuffers(window); glfwPollEvents();
    }
    
    glDeleteVertexArrays(1, &VAO); glDeleteBuffers(1, &VBO);
    glfwTerminate(); return 0;
}