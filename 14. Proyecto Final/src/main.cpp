// =====================================================================
//  RUBIK PRO — juego profesional de Cubo Rubik + demo final del curso
//  Programacion 100% orientada a objetos con arquitectura MVC:
//    MODELO       Move, Cubie, RubiksCube, IKChain      (estado y reglas)
//    VISTA        Shader, Texture, Camera, Renderer     (como se dibuja)
//    CONTROLADOR  Animator, Game, App                   (que pasa y cuando)
//
//  MODOS:  J jugar (U D L R F B, SHIFT invierte) | M/1 demo automatica
//          2 path tracer Monte Carlo | 3 cinematica inversa | T toon NPR
// =====================================================================
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "modelo/RubiksCube.h"
#include "modelo/IKChain.h"
#include "controlador/Animator.h"
#include "controlador/Game.h"
#include "vista/Camera.h"
#include "vista/Renderer.h"
#include "vista/Hud.h"

// =========================== CONTROLADOR ===========================
// CLASE App: la aplicacion completa. Es duena de la ventana, crea las
// piezas del MVC, reparte los eventos de GLFW y ejecuta el bucle principal.
class App {
public:
    // Constructor: crea la ventana OpenGL 3.3 y las piezas del MVC.
    App() {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 4);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(1280, 720, "RUBIK PRO", NULL, NULL);
        glfwMakeContextCurrent(window);
        glfwSetWindowUserPointer(window, this);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, sMouse);
        glfwSetScrollCallback(window, sScroll);

        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);

        // --- construccion del MVC ---
        animator = new Animator(cube);
        game     = new Game(cube, *animator, camera, window);
        arm.init(5, 0.85f, glm::vec3(1.5f, 0.0f, 0.0f));
        int w, h; glfwGetFramebufferSize(window, &w, &h);
        renderer.init(w, h);
        hud.init(w, h);      // interfaz 2D: guia de teclas sobre la escena

        printHelp();
    }

    // Destructor: libera recursos de GPU y cierra GLFW.
    ~App() {
        hud.destroy();
        renderer.destroy();
        delete game; delete animator;
        glfwTerminate();
    }

    // Bucle principal del juego: entrada -> logica -> dibujo.
    void run() {
        float lastFrame = 0.0f;
        while (!glfwWindowShouldClose(window)) {
            float now = (float)glfwGetTime();
            float dt = now - lastFrame; lastFrame = now;
            if (dt > 0.1f) dt = 0.1f;

            // --- CONTROLADOR ---
            game->handleInput();
            animator->update(dt);
            game->updateRecords();
            game->updateTitle(now);
            camera.update(dt);

            // --- VISTA (segun el modo activo) ---
            switch (game->mode) {
            case Game::Mode::AUTO:
            case Game::Mode::PLAY:
                renderer.renderRubik(cube, *animator, camera, now, game->isToon());
                break;
            case Game::Mode::PATHTRACER:
                renderer.renderPathTracer(game->ptFrame, game->ptCur);
                break;
            case Game::Mode::IK: {
                // objetivo en curva de Lissajous (animacion procedural)
                glm::vec3 target = arm.base + glm::vec3(
                    1.9f * sin(now * 0.9f),
                    1.6f + 1.1f * sin(now * 1.35f),
                    1.7f * cos(now * 0.7f));
                arm.solve(target);            // MODELO: cinematica inversa
                renderer.renderIK(arm, target, camera, now, game->isToon());
                break; }
            }

            // --- VISTA 2D: la guia de teclas siempre encima de la escena ---
            hud.render(*game, *animator, now);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

private:
    GLFWwindow* window = nullptr;
    RubiksCube  cube;         // MODELO
    IKChain     arm;          // MODELO
    Animator*   animator;     // CONTROLADOR
    Game*       game;         // CONTROLADOR
    Camera      camera;       // VISTA
    Renderer    renderer;     // VISTA
    Hud         hud;          // VISTA (interfaz 2D con la guia de teclas)
    bool  firstMouse = true;
    float lastX = 0, lastY = 0;

    // Imprime la guia de controles en la consola.
    void printHelp() {
        std::cout <<
          "==================================================\n"
          "  RUBIK PRO - demo final de Computacion Grafica\n"
          "--------------------------------------------------\n"
          "  J        jugar: resuelve el cubo tu mismo\n"
          "           U D L R F B giran caras (SHIFT invierte)\n"
          "  M o 1    demo automatica (se arma solito)\n"
          "  2        path tracer Monte Carlo (ilum. global)\n"
          "  3        cinematica inversa + deformacion\n"
          "  T        Phong <-> Toon (NPR)     O  auto-orbita\n"
          "  ESPACIO  nueva partida / mezcla / reiniciar\n"
          "  H        muestra u oculta la ayuda EN PANTALLA\n"
          "  Mouse    orbitar    Scroll  zoom    ESC  salir\n"
          "==================================================\n";
    }

    // Callback de mouse: reenvia el evento a la camara de esta instancia.
    static void sMouse(GLFWwindow* w, double x, double y) {
        App* a = (App*)glfwGetWindowUserPointer(w);
        if (a->firstMouse) { a->lastX=(float)x; a->lastY=(float)y; a->firstMouse=false; }
        a->camera.onMouse((float)x - a->lastX, a->lastY - (float)y);
        a->lastX = (float)x; a->lastY = (float)y;
    }

    // Callback de la rueda: reenvia el zoom a la camara.
    static void sScroll(GLFWwindow* w, double, double dy) {
        ((App*)glfwGetWindowUserPointer(w))->camera.onScroll((float)dy);
    }
};

// Punto de entrada: crea la aplicacion y ejecuta el bucle principal.
int main() {
    App app;
    app.run();
    return 0;
}
