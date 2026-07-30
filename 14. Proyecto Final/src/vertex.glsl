#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in float aFaceID;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float uTime;
uniform float uDeform;   // 1 = deformacion procedural (bandera)

out vec3 FragPos;
out vec3 Normal;
flat out int FaceID;

void main() {
    vec3 p = aPos;
    // ANIMACION PROCEDURAL + DEFORMACION en el vertex shader:
    // onda seno cuya amplitud crece hacia el extremo libre (p.x)
    if (uDeform > 0.5)
        p.z += 0.18 * sin(p.x * 6.0 - uTime * 4.0) * p.x;

    FragPos = vec3(model * vec4(p, 1.0));
    Normal  = mat3(transpose(inverse(model))) * aNormal;
    FaceID  = int(aFaceID + 0.5);
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
