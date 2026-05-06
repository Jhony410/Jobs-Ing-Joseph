#version 330 core
layout (location = 0) in vec3 aPos;   // Posición del vértice
layout (location = 1) in vec3 aColor; // Color del vértice

out vec3 ourColor; // Pasamos el color al Fragment Shader

void main() {
    // Definimos la posición final en el espacio de recortes
    gl_Position = vec4(aPos, 1.0); 
    ourColor = aColor; // Enviamos el color sin cambios [cite: 3]
}