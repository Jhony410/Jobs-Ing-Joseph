#pragma once
#include <vector>
#include <random>
#include "Cubie.h"

// ============================== MODELO ==============================
// CLASE RubiksCube: el estado logico completo del cubo (27 piezas)
// y sus reglas: aplicar giros, generar mezclas, calcular la solucion
// y detectar cuando el cubo esta resuelto.
class RubiksCube {
public:
    std::vector<Cubie> cubies;

    // Constructor: crea el cubo en estado resuelto.
    RubiksCube() : rng(std::random_device{}()) { reset(); }

    // Regresa el cubo al estado resuelto original.
    void reset() {
        cubies.clear();
        for (int x = -1; x <= 1; x++)
            for (int y = -1; y <= 1; y++)
                for (int z = -1; z <= 1; z++) {
                    Cubie c;
                    c.init(glm::ivec3(x, y, z));
                    cubies.push_back(c);
                }
    }

    // Aplica un giro de capa a todas las piezas de esa capa.
    void applyMove(const Move& m) {
        for (auto& c : cubies)
            if (c.inLayer(m)) c.applyMove(m);
    }

    // Genera una mezcla aleatoria de n movimientos, evitando
    // repetir la misma capa dos veces seguidas.
    std::vector<Move> generateScramble(int n) {
        std::vector<Move> seq;
        std::uniform_int_distribution<int> dAxis(0, 2), dLayer(-1, 1), dDir(0, 1);
        while ((int)seq.size() < n) {
            Move m{ dAxis(rng), dLayer(rng), dDir(rng) ? 1 : -1 };
            if (!seq.empty()) {
                const Move& p = seq.back();
                if (p.axis == m.axis && p.layer == m.layer) continue;
            }
            seq.push_back(m);
        }
        return seq;
    }

    // Solucion garantizada de una mezcla: sus inversos en orden inverso.
    static std::vector<Move> solutionFor(const std::vector<Move>& scramble) {
        std::vector<Move> sol;
        for (auto it = scramble.rbegin(); it != scramble.rend(); ++it)
            sol.push_back(it->inverse());
        return sol;
    }

    // El cubo esta resuelto si todas las piezas estan en su sitio
    // con sus stickers apuntando al color correcto.
    bool isSolved() const {
        for (const auto& c : cubies)
            if (!c.isHome()) return false;
        return true;
    }

private:
    std::mt19937 rng;   // generador de numeros aleatorios de la mezcla
};
