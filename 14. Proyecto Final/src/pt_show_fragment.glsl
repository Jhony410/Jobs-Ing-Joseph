#version 330 core
// ---- Presentacion del path tracer ----
// Copia a la pantalla la textura donde se acumulan las muestras Monte Carlo.
// La imagen ya viene con tone mapping y gamma aplicados, asi que solo se
// muestra tal cual (se usa un quad y no glBlitFramebuffer porque la ventana
// tiene multimuestreo y el blit escalado no esta permitido en ese caso).
in vec2 vUV;
uniform sampler2D uImagen;
out vec4 FragColor;

void main() {
    FragColor = vec4(texture(uImagen, vUV).rgb, 1.0);
}
