#pragma once
#include <glad/glad.h>
#include <vector>

// ============================== VISTA ==============================
// CLASE Texture: textura de tablero generada por codigo, sin archivos.
// Demuestra glTexImage2D, filtros, modos de repeticion y mipmaps.
class Texture {
public:
    unsigned int ID = 0;   // identificador de la textura en OpenGL

    // Genera en memoria un tablero de ajedrez y lo sube a la GPU.
    void createChecker(int size = 256, int cells = 8) {
        std::vector<unsigned char> px(size * size * 3);
        int cell = size / cells;
        for (int y = 0; y < size; y++)
            for (int x = 0; x < size; x++) {
                bool a = ((x / cell) + (y / cell)) % 2 == 0;
                int i = (y * size + x) * 3;
                px[i]   = a ? 46 : 26;
                px[i+1] = a ? 52 : 30;
                px[i+2] = a ? 74 : 42;
            }
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, px.data());
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
};
