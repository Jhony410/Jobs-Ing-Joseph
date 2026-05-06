#version 330 core
out vec4 FragColor;
in vec3 ourColor; // Color recibido desde el Vertex Shader

void main() {
    // Asignamos el color final con opacidad 1.0 (sólido) [cite: 1]
    FragColor = vec4(ourColor, 1.0f);
}