#pragma once
#include <GLFW/glfw3.h>
#include <cstdio>
#include "../modelo/RubiksCube.h"
#include "../modelo/IKChain.h"
#include "Animator.h"
#include "../vista/Camera.h"

// =========================== CONTROLADOR ===========================
// CLASE Game: el "director" del juego. Administra los modos de la
// aplicacion, traduce el teclado del jugador a giros del cubo,
// lleva el record de la sesion y pinta el HUD en la barra de titulo.
class Game {
public:
    // Modos disponibles de la aplicacion.
    enum class Mode { AUTO, PLAY, PATHTRACER, IK };

    Mode mode = Mode::AUTO;
    int  ptFrame = 1, ptCur = 0;      // estado del acumulador Monte Carlo
    float bestTime = -1.0f;           // record de la sesion (segundos)
    int   bestMoves = -1;             // record de movimientos
    bool  showHelp = true;            // tarjeta de controles visible en pantalla

    // Constructor: enlaza el modelo, el animador, la camara y la ventana.
    Game(RubiksCube& cube, Animator& anim, Camera& cam, GLFWwindow* win)
        : cube(cube), anim(anim), cam(cam), window(win) {}

    // Procesa todo el teclado una vez por frame (con anti-rebote).
    void handleInput() {
        if (key(GLFW_KEY_ESCAPE, 0)) glfwSetWindowShouldClose(window, true);

        // ---- cambio de modo ----
        if (key(GLFW_KEY_J, 1)) { mode = Mode::PLAY; anim.startGame(); }
        if (key(GLFW_KEY_M, 2) || key(GLFW_KEY_1, 3)) { mode = Mode::AUTO; anim.startAuto(); }
        if (key(GLFW_KEY_2, 4)) { mode = Mode::PATHTRACER; ptFrame = 1; }
        if (key(GLFW_KEY_3, 5)) mode = Mode::IK;
        if (key(GLFW_KEY_T, 6)) toon = !toon;
        if (key(GLFW_KEY_O, 7)) cam.autoOrbit = !cam.autoOrbit;
        if (key(GLFW_KEY_H, 15)) showHelp = !showHelp;   // mostrar/ocultar ayuda
        if (key(GLFW_KEY_SPACE, 8)) {
            if (mode == Mode::PLAY) anim.startGame();       // nueva partida
            if (mode == Mode::AUTO) anim.startAuto();       // nueva demo
            if (mode == Mode::PATHTRACER) ptFrame = 1;      // reiniciar acumulacion
        }

        // ---- giros del jugador (notacion estandar U D L R F B) ----
        if (mode == Mode::PLAY) {
            bool inv = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)  == GLFW_PRESS
                    || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
            playerKey(GLFW_KEY_U,  9, {1,  1, -1}, inv);  // Up
            playerKey(GLFW_KEY_D, 10, {1, -1,  1}, inv);  // Down
            playerKey(GLFW_KEY_R, 11, {0,  1, -1}, inv);  // Right
            playerKey(GLFW_KEY_L, 12, {0, -1,  1}, inv);  // Left
            playerKey(GLFW_KEY_F, 13, {2,  1, -1}, inv);  // Front
            playerKey(GLFW_KEY_B, 14, {2, -1,  1}, inv);  // Back
        }
    }

    // Al ganar una partida, actualiza los records de la sesion.
    void updateRecords() {
        if (mode == Mode::PLAY && anim.justWon()) {
            float t = anim.getPlayTime(); int m = anim.getMoveCount();
            if (bestTime < 0 || t < bestTime)  bestTime  = t;
            if (bestMoves < 0 || m < bestMoves) bestMoves = m;
        }
    }

    // Escribe el HUD (modo, cronometro, movimientos, record) en el titulo.
    void updateTitle(float now) {
        if (now - lastTitle < 0.15f) return;
        lastTitle = now;
        char t[256];
        switch (mode) {
        case Mode::AUTO:
            snprintf(t, 256, "RUBIK PRO | DEMO AUTO | J: jugar  2: Monte Carlo  3: IK  T: toon");
            break;
        case Mode::PLAY:
            if (anim.getState() == Animator::State::CELEBRATING)
                snprintf(t, 256,
                  "RUBIK PRO | RESUELTO en %d movs y %.1f s | RECORD: %d movs / %.1f s | ESPACIO: otra",
                  anim.getMoveCount(), anim.getPlayTime(), bestMoves, bestTime);
            else if (anim.getState() == Animator::State::SCRAMBLING)
                snprintf(t, 256, "RUBIK PRO | MEZCLANDO...");
            else
                snprintf(t, 256,
                  "RUBIK PRO | JUGANDO | Movs: %d | %.1f s | U D L R F B (SHIFT invierte) | M: menu",
                  anim.getMoveCount(), anim.getPlayTime());
            break;
        case Mode::PATHTRACER:
            snprintf(t, 256, "RUBIK PRO | PATH TRACER MONTE CARLO | muestra %d | ESPACIO reinicia | M: menu", ptFrame);
            break;
        case Mode::IK:
            snprintf(t, 256, "RUBIK PRO | CINEMATICA INVERSA (FABRIK) + deformacion | T: toon | M: menu");
            break;
        }
        glfwSetWindowTitle(window, t);
    }

    // El modo NPR (toon) esta activo?
    bool isToon() const { return toon; }

private:
    RubiksCube& cube;      // MODELO
    Animator&   anim;      // CONTROLADOR de animacion
    Camera&     cam;       // VISTA (camara)
    GLFWwindow* window;    // ventana para leer teclado y poner el titulo
    bool held[20] = {};    // estado anterior de cada tecla (anti-rebote)
    bool toon = false;
    float lastTitle = -1.0f;

    // Devuelve true solo en el frame en que la tecla se presiona.
    bool key(int k, int slot) {
        bool p = glfwGetKey(window, k) == GLFW_PRESS;
        bool fresh = p && !held[slot];
        held[slot] = p;
        return fresh;
    }

    // Encola el giro del jugador; SHIFT invierte el sentido.
    void playerKey(int k, int slot, Move m, bool inv) {
        if (key(k, slot)) {
            if (inv) m.dir = -m.dir;
            anim.enqueuePlayerMove(m);
        }
    }
};
