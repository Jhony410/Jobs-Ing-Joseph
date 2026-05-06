#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>

// ─────────────────────────────────────────────
//  Compilación y enlazado de shaders
// ─────────────────────────────────────────────
unsigned int crearShaderDesdeArchivo(const char* rutaVertex, const char* rutaFragment) {
    std::ifstream vFile(rutaVertex), fFile(rutaFragment);
    std::stringstream vSS, fSS;
    vSS << vFile.rdbuf();
    fSS << fFile.rdbuf();
    std::string vCode = vSS.str(), fCode = fSS.str();
    const char* vSrc = vCode.c_str();
    const char* fSrc = fCode.c_str();

    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vSrc, NULL);
    glCompileShader(vs);

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fSrc, NULL);
    glCompileShader(fs);

    unsigned int prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

// Shader en código fuente (sin archivos externos) — vértice
const char* vertSrc = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;
out vec3 vColor;
uniform vec2 uOffset;
uniform float uScale;
void main() {
    vec2 p = aPos * uScale + uOffset;
    gl_Position = vec4(p, 0.0, 1.0);
    vColor = aColor;
}
)";

// Shader en código fuente — fragmento
const char* fragSrc = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;
uniform vec3 uSolidColor;
uniform bool uUseSolid;
void main() {
    if (uUseSolid)
        FragColor = vec4(uSolidColor, 1.0);
    else
        FragColor = vec4(vColor, 1.0);
}
)";

// ─────────────────────────────────────────────
//  Creación de shaders desde strings en memoria
// ─────────────────────────────────────────────
unsigned int crearShader() {
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertSrc, NULL);
    glCompileShader(vs);

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragSrc, NULL);
    glCompileShader(fs);

    unsigned int prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

// ─────────────────────────────────────────────
//  Helpers para crear VAO/VBO/EBO
// ─────────────────────────────────────────────
struct Mesh {
    unsigned int VAO, VBO, EBO;
    int indexCount;
};

Mesh crearMesh(float* verts, int vSize, unsigned int* idx, int iCount) {
    Mesh m;
    m.indexCount = iCount;
    glGenVertexArrays(1, &m.VAO);
    glGenBuffers(1, &m.VBO);
    glGenBuffers(1, &m.EBO);

    glBindVertexArray(m.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
    glBufferData(GL_ARRAY_BUFFER, vSize, verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, iCount * sizeof(unsigned int), idx, GL_STATIC_DRAW);

    // Atrib 0: posición XY
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Atrib 1: color RGB
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    return m;
}

// Mesh solo XY (sin color por vértice — usa uSolidColor)
Mesh crearMeshSimple(float* verts, int vSize, unsigned int* idx, int iCount) {
    Mesh m;
    m.indexCount = iCount;
    glGenVertexArrays(1, &m.VAO);
    glGenBuffers(1, &m.VBO);
    glGenBuffers(1, &m.EBO);

    glBindVertexArray(m.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
    glBufferData(GL_ARRAY_BUFFER, vSize, verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, iCount * sizeof(unsigned int), idx, GL_STATIC_DRAW);

    // Solo posición XY (stride = 2 floats)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    return m;
}

void dibujarMesh(const Mesh& m, unsigned int shader,
                 float ox, float oy, float scale,
                 bool solid, float r, float g, float b,
                 GLenum modo = GL_TRIANGLES) {
    glUseProgram(shader);
    glUniform2f(glGetUniformLocation(shader, "uOffset"), ox, oy);
    glUniform1f(glGetUniformLocation(shader, "uScale"),  scale);
    glUniform1i(glGetUniformLocation(shader, "uUseSolid"), solid ? 1 : 0);
    glUniform3f(glGetUniformLocation(shader, "uSolidColor"), r, g, b);
    glBindVertexArray(m.VAO);
    glDrawElements(modo, m.indexCount, GL_UNSIGNED_INT, 0);
}

// ─────────────────────────────────────────────
//  Generador de círculo / elipse
// ─────────────────────────────────────────────
struct CircleMesh {
    unsigned int VAO, VBO;
    int count;
};

CircleMesh crearCirculo(float cx, float cy, float rx, float ry, int segs) {
    int total = segs + 2;
    float* verts = new float[total * 2];
    verts[0] = cx; verts[1] = cy;
    for (int i = 0; i <= segs; i++) {
        float a = (float)i / segs * 2.0f * 3.14159265f;
        verts[(i + 1) * 2 + 0] = cx + cosf(a) * rx;
        verts[(i + 1) * 2 + 1] = cy + sinf(a) * ry;
    }
    CircleMesh m;
    m.count = total;
    glGenVertexArrays(1, &m.VAO);
    glGenBuffers(1, &m.VBO);
    glBindVertexArray(m.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
    glBufferData(GL_ARRAY_BUFFER, total * 2 * sizeof(float), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    delete[] verts;
    return m;
}

void dibujarCirculo(const CircleMesh& c, unsigned int shader,
                    float ox, float oy, float scale,
                    float r, float g, float b) {
    glUseProgram(shader);
    glUniform2f(glGetUniformLocation(shader, "uOffset"), ox, oy);
    glUniform1f(glGetUniformLocation(shader, "uScale"),  scale);
    glUniform1i(glGetUniformLocation(shader, "uUseSolid"), 1);
    glUniform3f(glGetUniformLocation(shader, "uSolidColor"), r, g, b);
    glBindVertexArray(c.VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, c.count);
}

// ─────────────────────────────────────────────
//  main
// ─────────────────────────────────────────────
int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Delfín - Atardecer", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    unsigned int shader = crearShader();

    // ── FONDO: degradado cielo atardecer (cenit oscuro → horizonte rojizo) ──
    // Formato: X, Y, R, G, B
    float bgVerts[] = {
        // Horizonte: morado-rojizo cálido
        -1.0f, -1.0f,   0.32f, 0.07f, 0.12f,
         1.0f, -1.0f,   0.32f, 0.07f, 0.12f,
        // Cenit: púrpura muy oscuro casi negro
         1.0f,  1.0f,   0.07f, 0.01f, 0.11f,
        -1.0f,  1.0f,   0.07f, 0.01f, 0.11f,
    };
    unsigned int bgIdx[] = { 0,1,2, 0,2,3 };
    Mesh bgMesh = crearMesh(bgVerts, sizeof(bgVerts), bgIdx, 6);

    // ── OCÉANO: tonos oscuros cálidos de atardecer ────────────────────────
    float seaVerts[] = {
        -1.0f, -1.0f,   0.05f, 0.02f, 0.01f,  // profundidades: casi negro
         1.0f, -1.0f,   0.05f, 0.02f, 0.01f,
         1.0f, -0.10f,  0.20f, 0.09f, 0.03f,  // superficie: marrón cálido oscuro
        -1.0f, -0.10f,  0.20f, 0.09f, 0.03f,
    };
    unsigned int seaIdx[] = { 0,1,2, 0,2,3 };
    Mesh seaMesh = crearMesh(seaVerts, sizeof(seaVerts), seaIdx, 6);

    // ── SOL: cuatro círculos concéntricos (efecto atardecer) ──────────────
    // Centro del sol: (0.42, -0.10) — semisumergido en el horizonte
    // ry = rx * (800/600) para que se vea circular en ventana 4:3
    CircleMesh sunRing1 = crearCirculo(0.42f, -0.10f, 0.36f, 0.48f, 64);  // anillo externo rosa-morado
    CircleMesh sunRing2 = crearCirculo(0.42f, -0.10f, 0.25f, 0.33f, 64);  // anillo medio rojo-naranja
    CircleMesh sunRing3 = crearCirculo(0.42f, -0.10f, 0.16f, 0.21f, 64);  // anillo interior naranja
    CircleMesh sunCore  = crearCirculo(0.42f, -0.10f, 0.09f, 0.12f, 64);  // núcleo blanco-amarillo
    CircleMesh sunReflect = crearCirculo(0.42f, -0.32f, 0.09f, 0.05f, 64); // reflejo dorado en el agua

    // ── OLAS: tiras de líneas (GL_LINE_STRIP) ─────────────────────────────
    const int OLA_SEGS = 40;
    float olaVerts[OLA_SEGS * 2];
    for (int i = 0; i < OLA_SEGS; i++) {
        float t = (float)i / (OLA_SEGS - 1);
        float x = -1.0f + t * 2.0f;
        float y = -0.10f + sinf(t * 6.28f * 1.5f) * 0.025f
                         + sinf(t * 6.28f * 3.0f) * 0.010f;
        olaVerts[i * 2 + 0] = x;
        olaVerts[i * 2 + 1] = y;
    }
    unsigned int olaIdx[OLA_SEGS];
    for (int i = 0; i < OLA_SEGS; i++) olaIdx[i] = i;
    Mesh olaMesh = crearMeshSimple(olaVerts, sizeof(olaVerts), olaIdx, OLA_SEGS);

    float ola2Verts[OLA_SEGS * 2];
    for (int i = 0; i < OLA_SEGS; i++) {
        float t = (float)i / (OLA_SEGS - 1);
        float x = -1.0f + t * 2.0f;
        float y = -0.16f + sinf(t * 6.28f * 2.0f + 0.8f) * 0.018f
                          + sinf(t * 6.28f * 4.0f) * 0.007f;
        ola2Verts[i * 2 + 0] = x;
        ola2Verts[i * 2 + 1] = y;
    }
    Mesh ola2Mesh = crearMeshSimple(ola2Verts, sizeof(ola2Verts), olaIdx, OLA_SEGS);

    float ola3Verts[OLA_SEGS * 2];
    for (int i = 0; i < OLA_SEGS; i++) {
        float t = (float)i / (OLA_SEGS - 1);
        float x = -1.0f + t * 2.0f;
        float y = -0.24f + sinf(t * 6.28f * 2.5f + 1.5f) * 0.012f;
        ola3Verts[i * 2 + 0] = x;
        ola3Verts[i * 2 + 1] = y;
    }
    Mesh ola3Mesh = crearMeshSimple(ola3Verts, sizeof(ola3Verts), olaIdx, OLA_SEGS);

    // ── DELFÍN: geometría original del enunciado (19 vértices) ─────────────
    // Formato: X, Y, R, G, B
    float dolphinVerts[] = {
        // v0: punta morro (dorso)
         0.90f,  0.04f,   0.15f, 0.52f, 0.82f,
        // v1: punta morro (vientre)
         0.88f, -0.05f,   0.85f, 0.93f, 0.97f,
        // v2: frente / melón
         0.65f,  0.26f,   0.12f, 0.48f, 0.78f,
        // v3: mentón
         0.63f, -0.14f,   0.85f, 0.93f, 0.97f,
        // v4: cuerpo superior frontal
         0.25f,  0.36f,   0.10f, 0.44f, 0.74f,
        // v5: cuerpo inferior frontal (vientre)
         0.22f, -0.22f,   0.82f, 0.91f, 0.96f,
        // v6: base frontal aleta dorsal
         0.08f,  0.40f,   0.08f, 0.40f, 0.70f,
        // v7: punta aleta dorsal
         0.03f,  0.74f,   0.05f, 0.33f, 0.63f,
        // v8: base trasera aleta dorsal
        -0.08f,  0.40f,   0.08f, 0.40f, 0.70f,
        // v9: punta aleta pectoral
         0.12f, -0.46f,   0.70f, 0.84f, 0.93f,
        // v10: cuerpo inferior medio (vientre)
        -0.12f, -0.28f,   0.82f, 0.91f, 0.96f,
        // v11: cuerpo superior trasero
        -0.42f,  0.26f,   0.08f, 0.39f, 0.68f,
        // v12: cuerpo inferior trasero (vientre)
        -0.42f, -0.20f,   0.78f, 0.88f, 0.94f,
        // v13: pedúnculo caudal (arriba)
        -0.75f,  0.10f,   0.07f, 0.35f, 0.65f,
        // v14: pedúnculo caudal (abajo)
        -0.75f, -0.09f,   0.07f, 0.35f, 0.65f,
        // v15: lóbulo superior aleta caudal
        -0.93f,  0.38f,   0.05f, 0.28f, 0.58f,
        // v16: escotadura aleta caudal (centro)
        -0.88f,  0.01f,   0.05f, 0.28f, 0.58f,
        // v17: lóbulo inferior aleta caudal
        -0.93f, -0.30f,   0.05f, 0.28f, 0.58f,
        // v18: centro cuerpo (hub)
         0.00f,  0.05f,   0.35f, 0.68f, 0.88f,
    };

    unsigned int dolphinIdx[] = {
         0,  2,  1,
         1,  2,  3,
         2,  4,  3,
         3,  4,  5,
        18,  4,  6,
        18,  6,  8,
        18,  8, 10,
        18, 10,  9,
        18,  9,  5,
        18,  5,  4,
         6,  7,  8,
         8, 11, 10,
        11, 12, 10,
        11, 13, 12,
        12, 13, 14,
        13, 16, 14,
        13, 15, 16,
        14, 16, 17,
    };
    Mesh dolphinMesh = crearMesh(dolphinVerts, sizeof(dolphinVerts), dolphinIdx, 54);

    // ── CONTORNO del delfín ───────────────────────────────────────────────
    unsigned int contornoIdx[] = {
        // morro
        0, 2,  2, 1,  1, 3,
        // cuerpo superior
        2, 4,  4, 6,  6, 8,  8, 11,  11, 13,  13, 15,
        // cuerpo inferior
        3, 5,  5, 9,  9, 10,  10, 12,  12, 14,  14, 17,
        // aleta dorsal
        6, 7,  7, 8,
        // cola
        13, 15,  15, 16,  16, 17,
        // morro cierre
        0, 1,
    };
    Mesh contornoMesh = crearMesh(dolphinVerts, sizeof(dolphinVerts),
                                   contornoIdx, sizeof(contornoIdx)/sizeof(unsigned int));

    // ─────────────────────────────────────────────────────────────────────────
    //  Loop de renderizado
    // ─────────────────────────────────────────────────────────────────────────
    float time = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        time = (float)glfwGetTime();

        // Actualizar olas con animación suave
        float wavePhase = time * 0.6f;
        for (int i = 0; i < OLA_SEGS; i++) {
            float t = (float)i / (OLA_SEGS - 1);
            olaVerts[i * 2 + 1] = -0.10f
                + sinf(t * 6.28f * 1.5f + wavePhase) * 0.025f
                + sinf(t * 6.28f * 3.0f + wavePhase * 1.3f) * 0.010f;
            ola2Verts[i * 2 + 1] = -0.16f
                + sinf(t * 6.28f * 2.0f + 0.8f + wavePhase * 0.8f) * 0.018f
                + sinf(t * 6.28f * 4.0f + wavePhase) * 0.007f;
            ola3Verts[i * 2 + 1] = -0.24f
                + sinf(t * 6.28f * 2.5f + 1.5f + wavePhase * 0.5f) * 0.012f;
        }
        glBindBuffer(GL_ARRAY_BUFFER, olaMesh.VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(olaVerts), olaVerts);
        glBindBuffer(GL_ARRAY_BUFFER, ola2Mesh.VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ola2Verts), ola2Verts);
        glBindBuffer(GL_ARRAY_BUFFER, ola3Mesh.VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ola3Verts), ola3Verts);

        glClear(GL_COLOR_BUFFER_BIT);

        // 1. Fondo: cielo atardecer púrpura oscuro
        dibujarMesh(bgMesh, shader, 0,0,1, false, 0,0,0);

        // 2. Anillos del sol — de mayor a menor (externo → núcleo)
        dibujarCirculo(sunRing1, shader, 0,0,1, 0.44f, 0.14f, 0.18f);  // rosa-morado
        dibujarCirculo(sunRing2, shader, 0,0,1, 0.62f, 0.22f, 0.08f);  // rojo-naranja oscuro
        dibujarCirculo(sunRing3, shader, 0,0,1, 0.85f, 0.52f, 0.12f);  // naranja cálido
        dibujarCirculo(sunCore,  shader, 0,0,1, 0.97f, 0.97f, 0.72f);  // núcleo blanco-amarillo

        // 3. Océano encima del sol (recorta la mitad inferior)
        dibujarMesh(seaMesh, shader, 0,0,1, false, 0,0,0);

        // 4. Reflejo del sol: óvalo dorado en el agua
        dibujarCirculo(sunReflect, shader, 0,0,1, 0.45f, 0.28f, 0.04f);

        // 5. Olas — color gris-azulado sobre el océano oscuro
        glLineWidth(2.5f);
        dibujarMesh(olaMesh,  shader, 0,0,1, true,  0.38f, 0.50f, 0.65f, GL_LINE_STRIP);
        glLineWidth(1.8f);
        dibujarMesh(ola2Mesh, shader, 0,0,1, true,  0.35f, 0.46f, 0.60f, GL_LINE_STRIP);
        glLineWidth(1.4f);
        dibujarMesh(ola3Mesh, shader, 0,0,1, true,  0.32f, 0.42f, 0.56f, GL_LINE_STRIP);

        // 6. Delfín: posición sobre el horizonte con leve oscilación
        float dolphinY = sinf(time * 0.9f) * 0.03f + 0.00f;
        float dolphinScale = 0.48f;
        float dolphinX = -0.38f;

        // 6a. Relleno con colores por vértice
        dibujarMesh(dolphinMesh, shader, dolphinX, dolphinY, dolphinScale, false, 0,0,0);

        // 6b. Contorno azul marino oscuro
        glLineWidth(2.0f);
        dibujarMesh(contornoMesh, shader, dolphinX, dolphinY, dolphinScale,
                    true, 0.07f, 0.18f, 0.40f, GL_LINES);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Limpieza
    glDeleteVertexArrays(1, &bgMesh.VAO);       glDeleteBuffers(1, &bgMesh.VBO);       glDeleteBuffers(1, &bgMesh.EBO);
    glDeleteVertexArrays(1, &seaMesh.VAO);      glDeleteBuffers(1, &seaMesh.VBO);      glDeleteBuffers(1, &seaMesh.EBO);
    glDeleteVertexArrays(1, &dolphinMesh.VAO);  glDeleteBuffers(1, &dolphinMesh.VBO);  glDeleteBuffers(1, &dolphinMesh.EBO);
    glDeleteVertexArrays(1, &contornoMesh.VAO); glDeleteBuffers(1, &contornoMesh.VBO); glDeleteBuffers(1, &contornoMesh.EBO);
    glDeleteVertexArrays(1, &olaMesh.VAO);      glDeleteBuffers(1, &olaMesh.VBO);      glDeleteBuffers(1, &olaMesh.EBO);
    glDeleteVertexArrays(1, &ola2Mesh.VAO);     glDeleteBuffers(1, &ola2Mesh.VBO);     glDeleteBuffers(1, &ola2Mesh.EBO);
    glDeleteVertexArrays(1, &ola3Mesh.VAO);     glDeleteBuffers(1, &ola3Mesh.VBO);     glDeleteBuffers(1, &ola3Mesh.EBO);
    glDeleteVertexArrays(1, &sunRing1.VAO);     glDeleteBuffers(1, &sunRing1.VBO);
    glDeleteVertexArrays(1, &sunRing2.VAO);     glDeleteBuffers(1, &sunRing2.VBO);
    glDeleteVertexArrays(1, &sunRing3.VAO);     glDeleteBuffers(1, &sunRing3.VBO);
    glDeleteVertexArrays(1, &sunCore.VAO);      glDeleteBuffers(1, &sunCore.VBO);
    glDeleteVertexArrays(1, &sunReflect.VAO);   glDeleteBuffers(1, &sunReflect.VBO);

    glfwTerminate();
    return 0;
}
