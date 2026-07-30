#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include "Shader.h"

// ============================== VISTA ==============================
// CLASE Overlay: motor de dibujo 2D para la interfaz (HUD).
// Genera por codigo un atlas de fuente 5x7 (sin archivos ni librerias),
// acumula rectangulos y letras en un solo buffer de vertices y los pinta
// con una sola llamada de dibujo sobre la escena 3D ya renderizada.
//
// Todo se mide en PIXELES de pantalla: el shader usa una proyeccion
// ortografica con el origen en la esquina superior izquierda.
class Overlay {
public:
    // Ancho y alto de una letra dentro de la celda del atlas.
    static const int GW = 5, GH = 7;      // pixeles de la letra
    static const int CW = 6, CH = 8;      // pixeles de la celda (letra + aire)

    // Crea el shader, el atlas de la fuente y el buffer dinamico.
    void init(int screenW, int screenH) {
        width = screenW; height = screenH;
        shader.load("src/hud_vertex.glsl", "src/hud_fragment.glsl");
        buildFontAtlas();
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        // layout: posicion(2) + uv(2) + color(4) = 8 floats por vertice
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(2*sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(4*sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }

    // Libera los recursos de GPU del HUD.
    void destroy() {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteTextures(1, &fontTex);
    }

    int screenWidth()  const { return width; }
    int screenHeight() const { return height; }

    // ---------- API de dibujo ----------

    // Comienza un frame de interfaz: vacia la lista de vertices.
    void begin() { verts.clear(); }

    // Rectangulo solido (paneles, barras, teclas).
    void rect(float x, float y, float w, float h, glm::vec4 color) {
        quad(x, y, w, h, solidU, solidV, solidU, solidV, color);
    }

    // Marco de 1 o mas pixeles de grosor (borde de un panel o de una tecla).
    void frame(float x, float y, float w, float h, float t, glm::vec4 color) {
        rect(x, y, w, t, color);                 // arriba
        rect(x, y + h - t, w, t, color);         // abajo
        rect(x, y, t, h, color);                 // izquierda
        rect(x + w - t, y, t, h, color);         // derecha
    }

    // Ancho en pixeles que ocupara un texto con esa escala.
    float textWidth(const std::string& s, float scale) const {
        if (s.empty()) return 0.0f;
        return (float)s.size() * CW * scale - scale;
    }

    // Alto en pixeles de una linea de texto.
    float textHeight(float scale) const { return GH * scale; }

    // Dibuja un texto con su sombra (mejora la lectura sobre la escena 3D).
    void text(float x, float y, float scale, glm::vec4 color, const std::string& s) {
        drawString(x + scale, y + scale, scale, glm::vec4(0, 0, 0, color.a * 0.65f), s);
        drawString(x, y, scale, color, s);
    }

    // Texto centrado horizontalmente respecto de un punto.
    void textCentered(float cx, float y, float scale, glm::vec4 color, const std::string& s) {
        text(cx - textWidth(s, scale) * 0.5f, y, scale, color, s);
    }

    // Texto alineado a la derecha.
    void textRight(float rx, float y, float scale, glm::vec4 color, const std::string& s) {
        text(rx - textWidth(s, scale), y, scale, color, s);
    }

    // Panel translucido con borde: la base de todas las tarjetas del HUD.
    void panel(float x, float y, float w, float h, glm::vec4 fondo, glm::vec4 borde) {
        rect(x, y, w, h, fondo);
        frame(x, y, w, h, 2.0f, borde);
    }

    // Dibuja una "tecla" tipo teclado y devuelve su ancho en pixeles.
    // Si mide un ancho minimo, la tecla se estira para uniformar columnas.
    float keyCap(float x, float y, float scale, const std::string& label,
                 glm::vec4 fondo, glm::vec4 borde, glm::vec4 texto,
                 float minW = 0.0f) {
        float padX = 7.0f * scale * 0.5f + 4.0f;
        float w = textWidth(label, scale) + padX * 2.0f;
        if (w < minW) w = minW;
        float h = textHeight(scale) + 12.0f;
        rect(x, y, w, h, fondo);
        frame(x, y, w, h, 2.0f, borde);
        text(x + (w - textWidth(label, scale)) * 0.5f, y + (h - textHeight(scale)) * 0.5f,
             scale, texto, label);
        return w;
    }

    // Envia todo lo acumulado a la GPU y lo dibuja encima de la escena.
    void end() {
        if (verts.empty()) return;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        shader.use();
        // Ortografica con el eje Y hacia abajo (como una imagen).
        shader.setMat4("uProj", glm::ortho(0.0f, (float)width, (float)height, 0.0f,
                                           -1.0f, 1.0f));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fontTex);
        shader.setInt("uFont", 0);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float),
                     verts.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(verts.size() / 8));
        glBindVertexArray(0);

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }

private:
    Shader shader;
    unsigned int vao = 0, vbo = 0, fontTex = 0;
    std::vector<float> verts;             // lote de vertices del frame
    int width = 1280, height = 720;
    float solidU = 0.0f, solidV = 0.0f;   // texel opaco para los rectangulos
    int atlasW = 0, atlasH = 0;

    // Fuente 5x7 en columnas: cada letra son 5 bytes y cada bit es un pixel
    // (bit 0 = fila de arriba). Cubre ASCII 32..95 (espacio a subrayado).
    static const unsigned char* font5x7(int idx) {
        static const unsigned char F[64][5] = {
            {0x00,0x00,0x00,0x00,0x00}, // espacio
            {0x00,0x00,0x5F,0x00,0x00}, // !
            {0x00,0x07,0x00,0x07,0x00}, // "
            {0x14,0x7F,0x14,0x7F,0x14}, // #
            {0x24,0x2A,0x7F,0x2A,0x12}, // $
            {0x23,0x13,0x08,0x64,0x62}, // %
            {0x36,0x49,0x55,0x22,0x50}, // &
            {0x00,0x05,0x03,0x00,0x00}, // '
            {0x00,0x1C,0x22,0x41,0x00}, // (
            {0x00,0x41,0x22,0x1C,0x00}, // )
            {0x14,0x08,0x3E,0x08,0x14}, // *
            {0x08,0x08,0x3E,0x08,0x08}, // +
            {0x00,0x50,0x30,0x00,0x00}, // ,
            {0x08,0x08,0x08,0x08,0x08}, // -
            {0x00,0x60,0x60,0x00,0x00}, // .
            {0x20,0x10,0x08,0x04,0x02}, // /
            {0x3E,0x51,0x49,0x45,0x3E}, // 0
            {0x00,0x42,0x7F,0x40,0x00}, // 1
            {0x42,0x61,0x51,0x49,0x46}, // 2
            {0x21,0x41,0x45,0x4B,0x31}, // 3
            {0x18,0x14,0x12,0x7F,0x10}, // 4
            {0x27,0x45,0x45,0x45,0x39}, // 5
            {0x3C,0x4A,0x49,0x49,0x30}, // 6
            {0x01,0x71,0x09,0x05,0x03}, // 7
            {0x36,0x49,0x49,0x49,0x36}, // 8
            {0x06,0x49,0x49,0x29,0x1E}, // 9
            {0x00,0x36,0x36,0x00,0x00}, // :
            {0x00,0x56,0x36,0x00,0x00}, // ;
            {0x08,0x14,0x22,0x41,0x00}, // <
            {0x14,0x14,0x14,0x14,0x14}, // =
            {0x00,0x41,0x22,0x14,0x08}, // >
            {0x02,0x01,0x51,0x09,0x06}, // ?
            {0x32,0x49,0x79,0x41,0x3E}, // @
            {0x7E,0x11,0x11,0x11,0x7E}, // A
            {0x7F,0x49,0x49,0x49,0x36}, // B
            {0x3E,0x41,0x41,0x41,0x22}, // C
            {0x7F,0x41,0x41,0x22,0x1C}, // D
            {0x7F,0x49,0x49,0x49,0x41}, // E
            {0x7F,0x09,0x09,0x09,0x01}, // F
            {0x3E,0x41,0x49,0x49,0x7A}, // G
            {0x7F,0x08,0x08,0x08,0x7F}, // H
            {0x00,0x41,0x7F,0x41,0x00}, // I
            {0x20,0x40,0x41,0x3F,0x01}, // J
            {0x7F,0x08,0x14,0x22,0x41}, // K
            {0x7F,0x40,0x40,0x40,0x40}, // L
            {0x7F,0x02,0x0C,0x02,0x7F}, // M
            {0x7F,0x04,0x08,0x10,0x7F}, // N
            {0x3E,0x41,0x41,0x41,0x3E}, // O
            {0x7F,0x09,0x09,0x09,0x06}, // P
            {0x3E,0x41,0x51,0x21,0x5E}, // Q
            {0x7F,0x09,0x19,0x29,0x46}, // R
            {0x46,0x49,0x49,0x49,0x31}, // S
            {0x01,0x01,0x7F,0x01,0x01}, // T
            {0x3F,0x40,0x40,0x40,0x3F}, // U
            {0x1F,0x20,0x40,0x20,0x1F}, // V
            {0x3F,0x40,0x38,0x40,0x3F}, // W
            {0x63,0x14,0x08,0x14,0x63}, // X
            {0x07,0x08,0x70,0x08,0x07}, // Y
            {0x61,0x51,0x49,0x45,0x43}, // Z
            {0x00,0x7F,0x41,0x41,0x00}, // [
            {0x02,0x04,0x08,0x10,0x20}, // backslash
            {0x00,0x41,0x41,0x7F,0x00}, // ]
            {0x04,0x02,0x01,0x02,0x04}, // ^
            {0x40,0x40,0x40,0x40,0x40}, // _
        };
        return F[idx];
    }

    // Convierte un caracter en indice del atlas (las minusculas se
    // dibujan como mayusculas porque la fuente solo tiene un juego).
    static int glyphIndex(char c) {
        if (c >= 'a' && c <= 'z') c = (char)(c - 32);
        if (c < 32 || c > 95) c = '?';
        return c - 32;
    }

    // Construye la textura de la fuente: 16 letras por fila, 4 filas,
    // mas una celda extra totalmente opaca para pintar rectangulos.
    void buildFontAtlas() {
        const int COLS = 16, ROWS = 5;          // 4 filas de letras + 1 solida
        atlasW = COLS * CW; atlasH = ROWS * CH; // 96 x 40 pixeles
        std::vector<unsigned char> px(atlasW * atlasH, 0);

        for (int g = 0; g < 64; g++) {
            const unsigned char* col = font5x7(g);
            int cx = (g % COLS) * CW, cy = (g / COLS) * CH;
            for (int x = 0; x < GW; x++)
                for (int y = 0; y < GH; y++)
                    if ((col[x] >> y) & 1)
                        px[(cy + y) * atlasW + (cx + x)] = 255;
        }
        // Celda 64: bloque opaco usado por rect()/frame().
        int sx = 0, sy = 4 * CH;
        for (int y = 0; y < CH; y++)
            for (int x = 0; x < CW; x++)
                px[(sy + y) * atlasW + (sx + x)] = 255;
        solidU = (sx + CW * 0.5f) / atlasW;
        solidV = (sy + CH * 0.5f) / atlasH;

        glGenTextures(1, &fontTex);
        glBindTexture(GL_TEXTURE_2D, fontTex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);   // filas de 1 byte por texel
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, atlasW, atlasH, 0,
                     GL_RED, GL_UNSIGNED_BYTE, px.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    // Agrega dos triangulos (un rectangulo texturizado) al lote.
    void quad(float x, float y, float w, float h,
              float u0, float v0, float u1, float v1, glm::vec4 c) {
        float q[6][4] = {
            { x,     y,     u0, v0 }, { x + w, y,     u1, v0 }, { x + w, y + h, u1, v1 },
            { x + w, y + h, u1, v1 }, { x,     y + h, u0, v1 }, { x,     y,     u0, v0 }
        };
        for (int i = 0; i < 6; i++) {
            verts.push_back(q[i][0]); verts.push_back(q[i][1]);
            verts.push_back(q[i][2]); verts.push_back(q[i][3]);
            verts.push_back(c.r); verts.push_back(c.g);
            verts.push_back(c.b); verts.push_back(c.a);
        }
    }

    // Recorre la cadena y agrega un rectangulo texturizado por letra.
    void drawString(float x, float y, float scale, glm::vec4 color, const std::string& s) {
        for (char ch : s) {
            if (ch != ' ') {
                int g = glyphIndex(ch);
                float u0 = (g % 16) * (float)CW / atlasW;
                float v0 = (g / 16) * (float)CH / atlasH;
                quad(x, y, GW * scale, GH * scale,
                     u0, v0, u0 + (float)GW / atlasW, v0 + (float)GH / atlasH, color);
            }
            x += CW * scale;
        }
    }
};
