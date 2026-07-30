#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// ============================== MODELO ==============================
// CLASE Move: representa un giro de una capa del cubo.
// Guarda el eje de rotacion (0=X, 1=Y, 2=Z), la capa (-1, 0, 1)
// y el sentido del giro (+1 horario, -1 antihorario).
struct Move {
    int axis;
    int layer;
    int dir;

    // Devuelve el movimiento inverso (mismo eje y capa, sentido opuesto).
    Move inverse() const { return { axis, layer, -dir }; }

    // Devuelve el eje de rotacion como vector unitario.
    glm::vec3 axisVec() const {
        return glm::vec3(axis == 0, axis == 1, axis == 2);
    }

    // Construye la matriz del giro completo (90 grados) de este movimiento.
    glm::mat4 fullRotation() const {
        return glm::rotate(glm::mat4(1.0f), glm::radians(90.0f * dir), axisVec());
    }
};
