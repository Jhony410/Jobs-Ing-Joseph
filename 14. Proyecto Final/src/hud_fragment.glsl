#version 330 core
// ---- FRAGMENT SHADER DEL HUD ----
// El atlas de la fuente guarda solo la cobertura (canal rojo): 1 dentro de
// la letra y 0 fuera. Esa cobertura se usa como transparencia del color.
in vec2 vUV;
in vec4 vColor;
out vec4 FragColor;

uniform sampler2D uFont;

void main() {
    float cobertura = texture(uFont, vUV).r;
    float alpha = vColor.a * cobertura;
    if (alpha < 0.004) discard;            // descarta el fondo de la letra
    FragColor = vec4(vColor.rgb, alpha);
}
