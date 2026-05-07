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
#include <map>
#include <algorithm>

struct GraphNode { int id; double lon, lat; };
std::map<int, GraphNode> g_nodes;
std::map<int, std::vector<int>> g_adj;
std::vector<float> mapVertices; 

glm::vec3 targetPos = glm::vec3(0.0f, 0.0f, 0.0f); 
float radius = 1000.0f; 
float camYaw = -90.0f;  
float camPitch = 45.0f; 

float lastX = 512.0f;
float lastY = 384.0f;
bool firstMouse = true;

float deltaTime = 0.0f;	
float lastFrame = 0.0f;

void loadPunoTo3D() {
    std::ifstream f("calles_puno.csv");
    if (!f.is_open()) { std::cout << "Error leyendo CSV\n"; return; }
    
    std::string line; 
    std::getline(f, line); 
    
    double minLon = 180, maxLon = -180, minLat = 90, maxLat = -90;

    while (std::getline(f, line)) {
        size_t s = line.find('('), e = line.find(')');
        if (s == std::string::npos) continue;
        std::stringstream ss(line.substr(s + 1, e - s - 1));
        std::string pt; int prev = -1;
        
        while (std::getline(ss, pt, ',')) {
            std::stringstream ss2(pt); double lon, lat;
            if (ss2 >> lon >> lat) {
                int id = (int)g_nodes.size();
                g_nodes[id] = { id, lon, lat };
                minLon = (std::min)(minLon, lon); maxLon = (std::max)(maxLon, lon);
                minLat = (std::min)(minLat, lat); maxLat = (std::max)(maxLat, lat);
                
                if (prev != -1) { 
                    g_adj[prev].push_back(id); 
                }
                prev = id;
            }
        }
    }

    double cx = (minLon + maxLon) / 2.0;
    double cy = (minLat + maxLat) / 2.0;
    float scale = 20000.0f; 
    
    float halfWidth = 0.4f; 

    for (auto& pair : g_adj) {
        int u = pair.first;
        glm::vec2 p1((g_nodes[u].lon - cx) * scale, -(g_nodes[u].lat - cy) * scale);

        for (int v : pair.second) {
            glm::vec2 p2((g_nodes[v].lon - cx) * scale, -(g_nodes[v].lat - cy) * scale);

            glm::vec2 dir = p2 - p1;
            float length = glm::length(dir);
            if(length < 0.0001f) continue; 
            dir /= length; 

            glm::vec2 perp(-dir.y, dir.x);

            glm::vec2 c1 = p1 + perp * halfWidth; 
            glm::vec2 c2 = p1 - perp * halfWidth; 
            glm::vec2 c3 = p2 + perp * halfWidth; 
            glm::vec2 c4 = p2 - perp * halfWidth; 

            mapVertices.push_back(c1.x); mapVertices.push_back(0.0f); mapVertices.push_back(c1.y);
            mapVertices.push_back(c2.x); mapVertices.push_back(0.0f); mapVertices.push_back(c2.y);
            mapVertices.push_back(c3.x); mapVertices.push_back(0.0f); mapVertices.push_back(c3.y);

            mapVertices.push_back(c2.x); mapVertices.push_back(0.0f); mapVertices.push_back(c2.y);
            mapVertices.push_back(c4.x); mapVertices.push_back(0.0f); mapVertices.push_back(c4.y);
            mapVertices.push_back(c3.x); mapVertices.push_back(0.0f); mapVertices.push_back(c3.y);
        }
    }
    std::cout << "Calles solidas generadas. Vertices: " << mapVertices.size() << "\n";
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn); float ypos = static_cast<float>(yposIn);
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    
    float xoffset = xpos - lastX; float yoffset = lastY - ypos; 
    lastX = xpos; lastY = ypos;

    float sensitivity = 0.2f;
    camYaw += xoffset * sensitivity; 
    camPitch += yoffset * sensitivity;

    if (camPitch > 89.0f) camPitch = 89.0f;
    if (camPitch < 5.0f) camPitch = 5.0f; 
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    radius -= static_cast<float>(yoffset) * 100.0f; 
    if (radius < 10.0f) radius = 10.0f;       
    if (radius > 20000.0f) radius = 20000.0f;   
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float panSpeed = 500.0f * deltaTime; 
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) panSpeed *= 3.0f;

    glm::vec3 flatFront = glm::normalize(glm::vec3(cos(glm::radians(camYaw)), 0.0f, sin(glm::radians(camYaw))));
    glm::vec3 flatRight = glm::normalize(glm::cross(flatFront, glm::vec3(0.0f, 1.0f, 0.0f)));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) targetPos += flatFront * panSpeed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) targetPos -= flatFront * panSpeed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) targetPos -= flatRight * panSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) targetPos += flatRight * panSpeed;
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
    glAttachShader(ID, vertex); glAttachShader(ID, fragment); glLinkProgram(ID);
    glDeleteShader(vertex); glDeleteShader(fragment);
    return ID;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1024, 768, "Puno 3D asfaltaddo", NULL, NULL);
    glfwMakeContextCurrent(window);
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback); // ¡Vuelve el zoom!

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_DEPTH_TEST);

    loadPunoTo3D();

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, mapVertices.size() * sizeof(float), mapVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int shaderProgram = crearProgramaShader("src/vertex.glsl", "src/fragment.glsl");

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        float camX = targetPos.x + radius * cos(glm::radians(camPitch)) * cos(glm::radians(camYaw));
        float camY = targetPos.y + radius * sin(glm::radians(camPitch));
        float camZ = targetPos.z + radius * cos(glm::radians(camPitch)) * sin(glm::radians(camYaw));
        glm::vec3 cameraPos = glm::vec3(camX, camY, camZ);
        
        glm::mat4 view = glm::lookAt(cameraPos, targetPos, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1024.0f / 768.0f, 0.1f, 30000.0f); 
        glm::mat4 model = glm::mat4(1.0f); 

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, mapVertices.size() / 3); 

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}