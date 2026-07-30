#version 330 core
in vec3 FragPos;
in vec3 Normal;
flat in int FaceID;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 faceColors[6];
uniform float uGlow;
uniform float uMode;     // 0 Phong | 1 Toon (NPR)
uniform float uTexMode;  // 0 color | 1 textura tablero (plano XZ) | 2 franjas
uniform float uDeform;
uniform sampler2D uTex;

out vec4 FragColor;

void main() {
    vec3 base = faceColors[FaceID];
    if (uTexMode > 0.5 && uTexMode < 1.5)
        base = texture(uTex, FragPos.xz * 0.35).rgb;      // MAPEO DE TEXTURA
    else if (uTexMode > 1.5)
        base = mix(vec3(0.9,0.15,0.15), vec3(0.95),       // franjas de bandera
                   step(0.5, fract(FragPos.y * 3.0)));

    vec3 N = normalize(Normal);
    // normal recalculada para la malla deformada (derivadas de pantalla)
    if (uDeform > 0.5)
        N = normalize(cross(dFdx(FragPos), dFdy(FragPos)));

    vec3 V = normalize(viewPos - FragPos);
    if (dot(N, V) < 0.0) N = -N;                          // doble cara
    vec3 L = normalize(lightPos - FragPos);
    vec3 R = reflect(-L, N);
    float d = max(dot(N, L), 0.0);

    vec3 color;
    if (uMode > 0.5) {
        // ---- NPR: toon / cel shading ----
        float dq = floor(d * 3.0) / 3.0 + 0.25;
        color = dq * base;
        if (dot(N, V) < 0.30) color = vec3(0.0);
        if (pow(max(dot(V,R),0.0), 64.0) > 0.5) color += vec3(0.8);
    } else {
        // ---- Phong: ambiente + difusa + especular + rim ----
        color = 0.22 * base + d * base
              + pow(max(dot(V, R), 0.0), 48.0) * vec3(0.9)
              + pow(1.0 - max(dot(N, V), 0.0), 3.0) * vec3(0.25,0.45,0.9) * 0.6;
    }
    color += base * uGlow * 0.45;
    FragColor = vec4(color, 1.0);
}
