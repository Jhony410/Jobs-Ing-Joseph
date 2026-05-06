#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D miTextura; // Variable para la imagen

void main() {
    // Leemos el píxel de la imagen usando las coordenadas UV
    FragColor = texture(miTextura, TexCoord);
}