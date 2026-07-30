#pragma once
#include <vector>
#include <glm/glm.hpp>

// ============================== MODELO ==============================
// CLASE IKChain: cadena de articulaciones con cinematica inversa.
// Dado un punto objetivo, el algoritmo FABRIK calcula las posiciones
// de las articulaciones manteniendo la base fija y los largos constantes.
class IKChain {
public:
    std::vector<glm::vec3> joints;  // posiciones de las articulaciones
    glm::vec3 base;                 // punto de anclaje fijo
    float segLen;                   // longitud de cada segmento

    // Inicializa la cadena: n segmentos apilados verticalmente sobre la base.
    void init(int n, float len, const glm::vec3& b) {
        base = b; segLen = len;
        joints.resize(n + 1);
        for (int i = 0; i <= n; i++)
            joints[i] = b + glm::vec3(0, len * i, 0);
    }

    // Resuelve la cinematica inversa hacia el objetivo con FABRIK:
    // alterna pasadas hacia atras (efector al objetivo) y hacia
    // adelante (base a su sitio) hasta converger.
    void solve(const glm::vec3& target, int iters = 10) {
        int n = (int)joints.size() - 1;
        float reach = segLen * n;

        // Si el objetivo esta fuera de alcance, la cadena se estira recta.
        if (glm::distance(base, target) >= reach) {
            glm::vec3 d = glm::normalize(target - base);
            for (int i = 0; i <= n; i++) joints[i] = base + d * (segLen * i);
            return;
        }
        for (int it = 0; it < iters; it++) {
            joints[n] = target;
            for (int i = n - 1; i >= 0; i--) {
                glm::vec3 d = glm::normalize(joints[i] - joints[i + 1]);
                joints[i] = joints[i + 1] + d * segLen;
            }
            joints[0] = base;
            for (int i = 1; i <= n; i++) {
                glm::vec3 d = glm::normalize(joints[i] - joints[i - 1]);
                joints[i] = joints[i - 1] + d * segLen;
            }
            if (glm::distance(joints[n], target) < 0.001f) break;
        }
    }
};
