#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 TexCoord;

// Matriz de transformación que recibiremos desde C++
uniform mat4 transformacion; 

void main() {
    // Multiplicamos la matriz por el vector de posición original
    gl_Position = transformacion * vec4(aPos.x, aPos.y, aPos.z, 1.0);
    ourColor = aColor;
    TexCoord = aTexCoord;
}