#version 330 core
// ---- VERTEX SHADER DEL HUD (texto e interfaz 2D) ----
// Trabaja en pixeles de pantalla: la matriz uProj (ortografica) convierte
// esas coordenadas al espacio de recorte de OpenGL.
layout (location = 0) in vec2 aPos;     // posicion en pixeles
layout (location = 1) in vec2 aUV;      // coordenada en el atlas de la fuente
layout (location = 2) in vec4 aColor;   // color RGBA del vertice

uniform mat4 uProj;

out vec2 vUV;
out vec4 vColor;

void main() {
    vUV     = aUV;
    vColor  = aColor;
    gl_Position = uProj * vec4(aPos, 0.0, 1.0);
}
