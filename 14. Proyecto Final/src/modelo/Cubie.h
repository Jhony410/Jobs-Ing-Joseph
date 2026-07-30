#pragma once
#include <glm/glm.hpp>
#include "Move.h"

// ============================== MODELO ==============================
// CLASE Cubie: una de las 27 piezas del cubo Rubik.
// Conoce su posicion logica en la rejilla, su posicion original,
// su orientacion acumulada y el color de sus 6 caras locales.
class Cubie {
public:
    glm::ivec3 gridPos;        // posicion actual en la rejilla (-1..1)
    glm::ivec3 homePos;        // posicion original (para saber si esta resuelto)
    glm::mat4  orientation;    // rotaciones acumuladas de la pieza
    glm::vec3  faceColors[6];  // stickers: +X,-X,+Y,-Y,+Z,-Z
    bool       exterior[6];    // true si esa cara es visible (tiene sticker)

    // Constructor: pieza en el origen sin rotaciones.
    Cubie() : gridPos(0), homePos(0), orientation(1.0f) {
        for (int i = 0; i < 6; i++) exterior[i] = false;
    }

    // Inicializa la pieza en su posicion original y asigna los
    // colores clasicos del cubo segun la cara que queda al exterior.
    void init(const glm::ivec3& p) {
        gridPos = homePos = p;
        orientation = glm::mat4(1.0f);

        const glm::vec3 INTERIOR(0.06f, 0.06f, 0.07f);
        const glm::vec3 ROJO    (0.85f, 0.10f, 0.12f);
        const glm::vec3 NARANJA (1.00f, 0.45f, 0.05f);
        const glm::vec3 BLANCO  (0.95f, 0.95f, 0.95f);
        const glm::vec3 AMARILLO(1.00f, 0.83f, 0.05f);
        const glm::vec3 VERDE   (0.05f, 0.65f, 0.20f);
        const glm::vec3 AZUL    (0.05f, 0.30f, 0.85f);

        exterior[0] = (p.x ==  1); faceColors[0] = exterior[0] ? ROJO     : INTERIOR;
        exterior[1] = (p.x == -1); faceColors[1] = exterior[1] ? NARANJA  : INTERIOR;
        exterior[2] = (p.y ==  1); faceColors[2] = exterior[2] ? BLANCO   : INTERIOR;
        exterior[3] = (p.y == -1); faceColors[3] = exterior[3] ? AMARILLO : INTERIOR;
        exterior[4] = (p.z ==  1); faceColors[4] = exterior[4] ? VERDE    : INTERIOR;
        exterior[5] = (p.z == -1); faceColors[5] = exterior[5] ? AZUL     : INTERIOR;
    }

    // Indica si esta pieza pertenece a la capa que gira el movimiento dado.
    bool inLayer(const Move& m) const { return gridPos[m.axis] == m.layer; }

    // Consolida un giro terminado: rota la posicion logica en la rejilla
    // y acumula la rotacion en la orientacion de la pieza.
    void applyMove(const Move& m) {
        glm::mat4 R = m.fullRotation();
        glm::vec4 np = R * glm::vec4(glm::vec3(gridPos), 1.0f);
        gridPos = glm::ivec3(glm::round(glm::vec3(np)));
        orientation = R * orientation;
    }

    // Devuelve la normal local de una de las 6 caras (0..5).
    static glm::ivec3 faceNormal(int f) {
        static const glm::ivec3 N[6] = {
            { 1,0,0}, {-1,0,0}, {0, 1,0}, {0,-1,0}, {0,0, 1}, {0,0,-1} };
        return N[f];
    }

    // La pieza esta "en su sitio": misma posicion original y cada
    // sticker apuntando en la direccion del color que le corresponde.
    bool isHome() const {
        if (gridPos != homePos) return false;
        for (int f = 0; f < 6; f++) {
            if (!exterior[f]) continue;
            glm::vec4 d = orientation * glm::vec4(glm::vec3(faceNormal(f)), 0.0f);
            if (glm::ivec3(glm::round(glm::vec3(d))) != faceNormal(f))
                return false;
        }
        return true;
    }
};
