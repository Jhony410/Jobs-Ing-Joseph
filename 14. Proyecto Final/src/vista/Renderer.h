#pragma once
#include <glad/glad.h>
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "../modelo/RubiksCube.h"
#include "../modelo/IKChain.h"
#include "../controlador/Animator.h"

// ============================== VISTA ==============================
// CLASE Renderer: toda la parte grafica de la aplicacion.
// Dibuja las escenas de la demo final:
//   1) Cubo Rubik (Phong o Toon) sobre un piso texturizado
//   2) Path tracer Monte Carlo (quad de pantalla + acumulacion)
//   3) Brazo con cinematica inversa + bandera procedural
class Renderer {
public:
    // Crea shaders, mallas, textura y buffers de acumulacion.
    void init(int fbW, int fbH) {
        shader.load("src/vertex.glsl", "src/fragment.glsl");
        ptShader.load("src/pt_vertex.glsl", "src/pt_fragment.glsl");
        ptShowShader.load("src/pt_vertex.glsl", "src/pt_show_fragment.glsl");
        checker.createChecker();
        buildCubeMesh();
        buildGridMesh();
        buildAccumBuffers();
        width = fbW; height = fbH;
    }

    // ---------- ESCENA 1: dibuja el cubo Rubik completo ----------
    void renderRubik(const RubiksCube& cube, const Animator& anim,
                     const Camera& cam, float time, bool toon) {
        beginFrame(cam, time);
        shader.setFloat("uMode", toon ? 1.0f : 0.0f);
        drawFloor(-1.9f);

        // Celebracion: giro global acelerado + latido + brillo pulsante
        glm::mat4 global(1.0f);
        float glow = 0.0f;
        if (anim.getState() == Animator::State::CELEBRATING) {
            float t = anim.celebrate();
            global = glm::rotate(global, glm::radians(t * t * 90.0f),
                                 glm::vec3(0.3f, 1.0f, 0.2f));
            global = glm::scale(global,
                     glm::vec3(1.0f + 0.06f * sin(t * 10.0f) * std::min(1.0f, t)));
            glow = 0.5f + 0.5f * sin(t * 6.0f);
        }
        shader.setFloat("uGlow", glow);

        // Cada cubie: (rotacion de capa animada) * traslacion * orientacion
        glBindVertexArray(cubeVAO);
        for (const auto& c : cube.cubies) {
            glm::mat4 model = global;
            if (anim.isAnimating() && c.inLayer(anim.currentMove()))
                model = model * glm::rotate(glm::mat4(1.0f),
                        glm::radians(anim.currentAngle()),
                        anim.currentMove().axisVec());
            model = model
                  * glm::translate(glm::mat4(1.0f), glm::vec3(c.gridPos) * 1.05f)
                  * c.orientation
                  * glm::scale(glm::mat4(1.0f), glm::vec3(0.985f));
            shader.setMat4("model", model);
            shader.setVec3Array("faceColors", c.faceColors, 6);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        shader.setFloat("uGlow", 0.0f);
    }

    // ---------- ESCENA 2: un paso del path tracer Monte Carlo ----------
    // Traza un camino nuevo por pixel, lo promedia con lo acumulado
    // (ping-pong de FBOs) y copia el resultado a la pantalla.
    void renderPathTracer(int& frame, int& cur) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo[cur]);
        glViewport(0, 0, PTW, PTH);
        glDisable(GL_DEPTH_TEST);
        ptShader.use();
        ptShader.setInt("uPrev", 0);
        glUniform1i(glGetUniformLocation(ptShader.ID, "uFrame"), frame);
        glUniform2f(glGetUniformLocation(ptShader.ID, "uRes"),
                    (float)PTW, (float)PTH);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, accumTex[1 - cur]);
        glBindVertexArray(emptyVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Presentacion: se dibuja la acumulacion en un quad de pantalla.
        // (No se usa glBlitFramebuffer porque la ventana es multimuestreada
        //  y OpenGL prohibe el blit ESCALADO hacia un destino con MSAA.)
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ptShowShader.use();
        ptShowShader.setInt("uImagen", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, accumTex[cur]);
        glBindSampler(0, linearSampler);      // suaviza al ampliar a la ventana
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindSampler(0, 0);

        glEnable(GL_DEPTH_TEST);
        cur = 1 - cur; frame++;
    }

    // ---------- ESCENA 3: brazo IK + bandera procedural ----------
    void renderIK(const IKChain& arm, const glm::vec3& target,
                  const Camera& cam, float time, bool toon) {
        beginFrame(cam, time);
        shader.setFloat("uMode", toon ? 1.0f : 0.0f);
        drawFloor(0.0f);

        glBindVertexArray(cubeVAO);
        // Huesos: cubos estirados y orientados entre articulaciones
        for (size_t i = 0; i + 1 < arm.joints.size(); i++)
            drawBone(arm.joints[i], arm.joints[i + 1],
                     glm::vec3(0.75f, 0.78f, 0.85f), 0.14f);
        // Articulaciones como cubitos naranjas
        for (const auto& j : arm.joints)
            drawBox(glm::translate(glm::mat4(1.0f), j)
                    * glm::scale(glm::mat4(1.0f), glm::vec3(0.24f)),
                    glm::vec3(1.0f, 0.55f, 0.1f));
        // Objetivo rojo que el brazo persigue
        drawBox(glm::translate(glm::mat4(1.0f), target)
                * glm::rotate(glm::mat4(1.0f), time * 2.0f, glm::vec3(0,1,0))
                * glm::scale(glm::mat4(1.0f), glm::vec3(0.22f)),
                glm::vec3(0.9f, 0.12f, 0.15f));

        // Bandera: la onda se calcula en el vertex shader (deformacion)
        shader.setFloat("uDeform", 1.0f);
        shader.setFloat("uTexMode", 2.0f);
        shader.setMat4("model",
            glm::translate(glm::mat4(1.0f), glm::vec3(-3.2f, 1.0f, -1.5f)));
        setSolidColor(glm::vec3(1.0f));
        glBindVertexArray(gridVAO);
        glDrawArrays(GL_TRIANGLES, 0, gridVerts);
        shader.setFloat("uDeform", 0.0f);
        shader.setFloat("uTexMode", 0.0f);
        // Mastil de la bandera
        glBindVertexArray(cubeVAO);
        drawBone(glm::vec3(-3.2f, 0.0f, -1.5f), glm::vec3(-3.2f, 2.2f, -1.5f),
                 glm::vec3(0.45f, 0.3f, 0.15f), 0.07f);
    }

    // Libera los recursos de GPU creados por la vista.
    void destroy() {
        glDeleteVertexArrays(1, &cubeVAO); glDeleteBuffers(1, &cubeVBO);
        glDeleteVertexArrays(1, &gridVAO); glDeleteBuffers(1, &gridVBO);
        glDeleteSamplers(1, &linearSampler);
    }

private:
    Shader shader, ptShader, ptShowShader;   // programas de GPU
    Texture checker;              // textura del piso
    unsigned int cubeVAO = 0, cubeVBO = 0;
    unsigned int gridVAO = 0, gridVBO = 0;
    unsigned int emptyVAO = 0;    // VAO vacio para el quad de pantalla
    unsigned int accumTex[2] = {0,0}, fbo[2] = {0,0};
    unsigned int linearSampler = 0;   // filtrado suave solo al presentar
    int gridVerts = 0, width = 1280, height = 720;
    static const int PTW = 960, PTH = 540;   // resolucion del path tracer

    // Prepara el frame: limpia pantalla y envia matrices, luz y camara.
    void beginFrame(const Camera& cam, float time) {
        glViewport(0, 0, width, height);
        glClearColor(0.045f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.use();
        shader.setMat4("view", cam.view());
        shader.setMat4("projection", glm::perspective(glm::radians(45.0f),
                       (float)width / (float)height, 0.1f, 100.0f));
        shader.setVec3("viewPos", cam.position());
        shader.setVec3("lightPos",
            glm::vec3(6.0f * sin(time * 0.7f), 5.0f, 6.0f * cos(time * 0.7f)));
        shader.setFloat("uTime", time);
        shader.setFloat("uGlow", 0.0f);
        shader.setFloat("uDeform", 0.0f);
        shader.setFloat("uTexMode", 0.0f);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, checker.ID);
        shader.setInt("uTex", 0);
    }

    // Asigna el mismo color a las 6 caras del cubo unitario.
    void setSolidColor(const glm::vec3& c) {
        glm::vec3 cols[6] = { c, c, c, c, c, c };
        shader.setVec3Array("faceColors", cols, 6);
    }

    // Dibuja el cubo unitario con la transformacion y color dados.
    void drawBox(const glm::mat4& model, const glm::vec3& color) {
        shader.setMat4("model", model);
        setSolidColor(color);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // Dibuja el piso: una losa grande con la textura de tablero.
    void drawFloor(float y) {
        shader.setFloat("uTexMode", 1.0f);
        glBindVertexArray(cubeVAO);
        drawBox(glm::translate(glm::mat4(1.0f), glm::vec3(0, y - 0.05f, 0))
                * glm::scale(glm::mat4(1.0f), glm::vec3(24.0f, 0.1f, 24.0f)),
                glm::vec3(1.0f));
        shader.setFloat("uTexMode", 0.0f);
    }

    // Dibuja un "hueso": cubo estirado y rotado a lo largo del segmento a->b.
    void drawBone(const glm::vec3& a, const glm::vec3& b,
                  const glm::vec3& color, float grosor) {
        glm::vec3 d = b - a;
        float len = glm::length(d);
        d /= len;
        glm::vec3 Y(0, 1, 0);
        glm::mat4 R(1.0f);
        float c = glm::dot(Y, d);
        if (c < 0.9999f && c > -0.9999f)
            R = glm::rotate(glm::mat4(1.0f), acosf(c),
                            glm::normalize(glm::cross(Y, d)));
        else if (c < 0.0f)
            R = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1,0,0));
        drawBox(glm::translate(glm::mat4(1.0f), (a + b) * 0.5f) * R
                * glm::scale(glm::mat4(1.0f), glm::vec3(grosor, len, grosor)),
                color);
    }

    // Construye la malla del cubo unitario: posicion, normal e id de cara.
    void buildCubeMesh() {
        float V[36 * 7]; int i = 0;
        auto quad = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d,
                        glm::vec3 n, float id) {
            glm::vec3 tri[6] = { a, b, c, c, d, a };
            for (auto& p : tri) { V[i++]=p.x; V[i++]=p.y; V[i++]=p.z;
                V[i++]=n.x; V[i++]=n.y; V[i++]=n.z; V[i++]=id; }
        };
        float s = 0.5f;
        quad({ s,-s,-s},{ s, s,-s},{ s, s, s},{ s,-s, s},{ 1,0,0},0);
        quad({-s,-s, s},{-s, s, s},{-s, s,-s},{-s,-s,-s},{-1,0,0},1);
        quad({-s, s,-s},{-s, s, s},{ s, s, s},{ s, s,-s},{0, 1,0},2);
        quad({-s,-s, s},{-s,-s,-s},{ s,-s,-s},{ s,-s, s},{0,-1,0},3);
        quad({-s,-s, s},{ s,-s, s},{ s, s, s},{-s, s, s},{0,0, 1},4);
        quad({ s,-s,-s},{-s,-s,-s},{-s, s,-s},{ s, s,-s},{0,0,-1},5);
        makeVAO(cubeVAO, cubeVBO, V, sizeof(V));
    }

    // Construye la malla plana subdividida para la bandera deformable.
    void buildGridMesh() {
        const int NX = 40, NY = 14;
        const float W = 1.7f, H = 0.9f;
        std::vector<float> V;
        auto push = [&](float x, float y) {
            V.push_back(x); V.push_back(y); V.push_back(0.0f);
            V.push_back(0); V.push_back(0); V.push_back(1);
            V.push_back(0.0f);
        };
        for (int y = 0; y < NY; y++)
            for (int x = 0; x < NX; x++) {
                float x0 = W * x / NX,      x1 = W * (x + 1) / NX;
                float y0 = H * y / NY,      y1 = H * (y + 1) / NY;
                push(x0,y0); push(x1,y0); push(x1,y1);
                push(x1,y1); push(x0,y1); push(x0,y0);
            }
        gridVerts = (int)V.size() / 7;
        makeVAO(gridVAO, gridVBO, V.data(), V.size() * sizeof(float));
    }

    // Crea un VAO + VBO con el layout comun (pos, normal, faceID).
    void makeVAO(unsigned int& vao, unsigned int& vbo, const float* data, size_t bytes) {
        glGenVertexArrays(1, &vao); glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, bytes, data, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7*sizeof(float), (void*)(6*sizeof(float)));
        glEnableVertexAttribArray(2);
    }

    // Crea las dos texturas flotantes y FBOs del acumulador Monte Carlo.
    void buildAccumBuffers() {
        glGenVertexArrays(1, &emptyVAO);
        glGenTextures(2, accumTex); glGenFramebuffers(2, fbo);
        for (int i = 0; i < 2; i++) {
            glBindTexture(GL_TEXTURE_2D, accumTex[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, PTW, PTH, 0,
                         GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, accumTex[i], 0);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Sampler aparte: la acumulacion se lee siempre con NEAREST (exacta),
        // pero la imagen final se muestra en pantalla con filtrado lineal.
        glGenSamplers(1, &linearSampler);
        glSamplerParameteri(linearSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glSamplerParameteri(linearSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(linearSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glSamplerParameteri(linearSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
};
