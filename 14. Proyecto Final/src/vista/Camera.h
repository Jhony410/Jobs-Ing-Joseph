#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// ============================== VISTA ==============================
// CLASE Camera: camara orbital alrededor del origen.
// Gira sola de forma cinematografica; al mover el mouse, el
// usuario toma el control. La rueda controla el zoom.
class Camera {
public:
    float yaw = 45.0f, pitch = 25.0f, radius = 9.0f;
    bool  autoOrbit = true;   // giro automatico activado

    // Avanza la orbita automatica si esta activada.
    void update(float dt) {
        if (autoOrbit) yaw += 10.0f * dt;
    }

    // Reacciona al movimiento del mouse: orbitar manualmente.
    void onMouse(float dx, float dy) {
        autoOrbit = false;
        yaw   += dx * 0.15f;
        pitch += dy * 0.15f;
        pitch = glm::clamp(pitch, -85.0f, 85.0f);
    }

    // Reacciona a la rueda del mouse: acercar o alejar.
    void onScroll(float dy) {
        radius = glm::clamp(radius - dy, 5.0f, 18.0f);
    }

    // Calcula la posicion de la camara en coordenadas esfericas.
    glm::vec3 position() const {
        float cy = cos(glm::radians(pitch)), sy = sin(glm::radians(pitch));
        return glm::vec3(sin(glm::radians(yaw)) * cy, sy,
                         cos(glm::radians(yaw)) * cy) * radius;
    }

    // Devuelve la matriz de vista (lookAt hacia el origen).
    glm::mat4 view() const {
        return glm::lookAt(position(), glm::vec3(0.0f), glm::vec3(0, 1, 0));
    }
};
