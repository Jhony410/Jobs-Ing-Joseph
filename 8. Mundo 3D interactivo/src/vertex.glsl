#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

// ¡NUEVO! El sagrado trío de matrices (MVP)
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // La multiplicación se lee de derecha a izquierda
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}