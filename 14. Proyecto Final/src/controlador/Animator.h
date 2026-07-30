#pragma once
#include <deque>
#include <cmath>
#include <algorithm>
#include "../modelo/RubiksCube.h"

// =========================== CONTROLADOR ===========================
// CLASE Animator: maquina de estados que gobierna la vida del cubo.
//  MODO AUTO:    MEZCLAR -> PAUSA -> RESOLVER (acelerando) -> CELEBRAR -> repite.
//  MODO JUGADOR: MEZCLAR -> JUGANDO (el usuario gira capas) -> al detectar
//                el cubo resuelto -> CELEBRAR. Lleva movimientos y cronometro.
// Anima cada giro interpolando el angulo con una curva de easing (keyframes).
class Animator {
public:
    enum class State { SCRAMBLING, PAUSED, SOLVING, PLAYING, CELEBRATING };

    // Constructor: guarda la referencia al modelo y arranca en modo auto.
    Animator(RubiksCube& cube) : cube(cube) { startAuto(); }

    // Inicia el ciclo automatico (modo demostracion / atraccion).
    void startAuto() {
        playerMode = false;
        cube.reset();
        beginScramble(22, 0.16f);
    }

    // Inicia una partida: mezcla el cubo y cede el control al jugador.
    void startGame() {
        playerMode = true;
        cube.reset();
        moveCount = 0; playTime = 0.0f; finished = false;
        beginScramble(22, 0.10f);   // mezcla rapida para empezar a jugar pronto
    }

    // Encola un giro pedido por el jugador (solo en estado JUGANDO).
    void enqueuePlayerMove(const Move& m) {
        if (state != State::PLAYING) return;
        if (queue.size() < 4) {     // pequenio buffer para fluidez
            queue.push_back(m);
            moveCount++;
            if (!animating) beginNextMove();
        }
    }

    // Avanza la logica un frame: anima giros, cambia de fase y cronometra.
    void update(float dt) {
        time += dt;
        switch (state) {
        case State::SCRAMBLING:
        case State::SOLVING:
            updateMove(dt);
            break;
        case State::PLAYING:
            playTime += dt;         // cronometro de la partida
            updateMove(dt);
            break;
        case State::PAUSED:
            if (time >= pauseUntil) {
                queue.assign(solution.begin(), solution.end());
                state = State::SOLVING;
                moveDuration = 0.55f;   // arranca lento y acelera
                beginNextMove();
            }
            break;
        case State::CELEBRATING:
            celebrateT += dt;
            // en modo auto la demo se reinicia sola; en modo jugador espera
            if (!playerMode && celebrateT > 5.0f) startAuto();
            break;
        }
    }

    // ---- Consultas que usa la Vista para dibujar ----

    // Hay un giro en curso ahora mismo?
    bool  isAnimating() const { return animating; }

    // El giro que se esta animando.
    Move  currentMove() const { return current; }

    // Angulo actual del giro en grados, suavizado con easing (keyframe).
    float currentAngle() const {
        float t = glm::clamp(progress, 0.0f, 1.0f);
        float e = t * t * (3.0f - 2.0f * t);
        return 90.0f * current.dir * e;
    }

    State getState()    const { return state; }
    bool  isPlayerMode()const { return playerMode; }
    float celebrate()   const { return celebrateT; }
    int   getMoveCount()const { return moveCount; }
    float getPlayTime() const { return playTime; }
    bool  justWon()     const { return finished; }

private:
    RubiksCube& cube;             // referencia al MODELO
    std::deque<Move> queue;       // giros pendientes de animar
    std::vector<Move> scramble, solution;
    State state = State::SCRAMBLING;
    Move  current{};
    bool  animating = false, playerMode = false, finished = false;
    float progress = 0.0f, moveDuration = 0.2f;
    float time = 0.0f, pauseUntil = 0.0f, celebrateT = 0.0f;
    int   moveCount = 0;
    float playTime = 0.0f;

    // Prepara y encola una mezcla nueva con la velocidad indicada.
    void beginScramble(int n, float speed) {
        scramble = cube.generateScramble(n);
        queue.assign(scramble.begin(), scramble.end());
        state = State::SCRAMBLING;
        moveDuration = speed;
        celebrateT = 0.0f;
        beginNextMove();
    }

    // Toma el siguiente giro de la cola y comienza a animarlo.
    void beginNextMove() {
        if (queue.empty()) { finishPhase(); return; }
        current = queue.front(); queue.pop_front();
        progress = 0.0f;
        animating = true;
    }

    // Avanza la animacion del giro actual; al llegar a 90 grados
    // lo consolida en el modelo y decide que sigue.
    void updateMove(float dt) {
        if (!animating) {
            // en modo jugador puede no haber nada que animar
            if (state == State::PLAYING && !queue.empty()) beginNextMove();
            return;
        }
        progress += dt / moveDuration;
        if (progress >= 1.0f) {
            cube.applyMove(current);                 // consolidar en el MODELO
            animating = false;
            if (state == State::SOLVING)             // aceleracion dramatica
                moveDuration = std::max(0.07f, moveDuration * 0.90f);
            if (state == State::PLAYING && queue.empty() && cube.isSolved()) {
                state = State::CELEBRATING;          // el jugador gano!
                celebrateT = 0.0f; finished = true;
                return;
            }
            beginNextMove();
        }
    }

    // Cierra la fase actual cuando la cola quedo vacia.
    void finishPhase() {
        if (state == State::SCRAMBLING) {
            if (playerMode) {
                state = State::PLAYING;              // turno del jugador
                moveDuration = 0.14f;                // giros agiles al jugar
            } else {
                solution = RubiksCube::solutionFor(scramble);
                state = State::PAUSED;
                pauseUntil = time + 1.2f;            // pausa de suspenso
            }
        } else if (state == State::SOLVING) {
            state = State::CELEBRATING;
            celebrateT = 0.0f;
        }
    }
};
