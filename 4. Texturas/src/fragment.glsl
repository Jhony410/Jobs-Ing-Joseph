#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D miTextura; // La imagen cargada

void main() {
    // texture() lee el píxel exacto de la imagen usando las coordenadas UV
    FragColor = texture(miTextura, TexCoord);
}