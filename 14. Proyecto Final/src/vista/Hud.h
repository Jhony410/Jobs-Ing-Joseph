#pragma once
#include <string>
#include <vector>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include "Overlay.h"
#include "../controlador/Game.h"
#include "../controlador/Animator.h"

// ============================== VISTA ==============================
// CLASE Hud: la interfaz que se dibuja ENCIMA de la escena 3D.
// Su trabajo es que el usuario nunca tenga que adivinar que tecla pulsar:
//   - cabecera con el titulo y el modo activo
//   - tarjeta de CONTROLES con todas las teclas (se muestra/oculta con H)
//   - fila de teclas U D L R F B mientras se juega
//   - barra de estado (movimientos, cronometro, record, muestras)
//   - mensajes grandes: MEZCLANDO / RESUELTO
class Hud {
public:
    // Crea el motor 2D con el tamanio de la ventana.
    void init(int w, int h) { ov.init(w, h); }

    // Libera los recursos del HUD.
    void destroy() { ov.destroy(); }

    // Dibuja la interfaz completa del frame actual.
    void render(const Game& game, const Animator& anim, float time) {
        W = (float)ov.screenWidth();
        H = (float)ov.screenHeight();
        ov.begin();
        drawHeader(game);
        helpRight = 24.0f;
        if (game.showHelp) drawControls(game);
        else               drawHint();
        if (game.mode == Game::Mode::PLAY) drawFaceKeys(anim, game.showHelp);
        drawStatusBar(game, anim);
        drawBigMessage(game, anim, time);
        ov.end();
    }

private:
    Overlay ov;
    float W = 1280.0f, H = 720.0f;
    float helpRight = 24.0f;   // borde derecho de la tarjeta de controles

    // --- paleta del HUD ---
    const glm::vec4 ACENTO { 0.30f, 0.85f, 1.00f, 1.00f };
    const glm::vec4 ORO    { 1.00f, 0.78f, 0.25f, 1.00f };
    const glm::vec4 BLANCO { 0.93f, 0.96f, 1.00f, 1.00f };
    const glm::vec4 GRIS   { 0.66f, 0.72f, 0.84f, 1.00f };
    const glm::vec4 FONDO  { 0.03f, 0.04f, 0.07f, 0.74f };
    const glm::vec4 BORDE  { 0.30f, 0.85f, 1.00f, 0.35f };
    const glm::vec4 CAPBG  { 0.10f, 0.13f, 0.21f, 0.94f };
    const glm::vec4 CAPBOR { 0.30f, 0.85f, 1.00f, 0.80f };

    // Nombre legible del modo activo (para la insignia de la cabecera).
    static const char* modeName(const Game& g) {
        switch (g.mode) {
        case Game::Mode::AUTO:       return "DEMO AUTOMATICA";
        case Game::Mode::PLAY:       return "JUGANDO";
        case Game::Mode::PATHTRACER: return "PATH TRACER";
        default:                     return "CINEMATICA INVERSA";
        }
    }

    // Que hace la barra ESPACIO en el modo actual.
    static const char* spaceAction(const Game& g) {
        switch (g.mode) {
        case Game::Mode::AUTO:       return "NUEVA MEZCLA";
        case Game::Mode::PLAY:       return "NUEVA PARTIDA";
        case Game::Mode::PATHTRACER: return "REINICIAR MUESTRAS";
        default:                     return "REINICIAR ESCENA";
        }
    }

    // ---------- CABECERA: titulo + insignia del modo ----------
    void drawHeader(const Game& game) {
        ov.text(26.0f, 22.0f, 5.0f, ACENTO, "RUBIK PRO");
        ov.text(28.0f, 66.0f, 2.0f, GRIS, "COMPUTACION GRAFICA - MVC + OPENGL");

        // Insignia del modo, pegada al borde derecho.
        std::string txt = std::string("MODO: ") + modeName(game);
        float tw = ov.textWidth(txt, 2.0f);
        float bw = tw + 28.0f, bh = 34.0f, bx = W - 24.0f - bw, by = 22.0f;
        ov.panel(bx, by, bw, bh, CAPBG, CAPBOR);
        ov.text(bx + 14.0f, by + (bh - ov.textHeight(2.0f)) * 0.5f, 2.0f, BLANCO, txt);

        // Estado del sombreado, debajo de la insignia.
        ov.textRight(W - 24.0f, by + bh + 10.0f, 2.0f, GRIS,
                     game.isToon() ? "SOMBREADO: TOON (NPR)" : "SOMBREADO: PHONG");
    }

    // ---------- TARJETA DE CONTROLES (se alterna con H) ----------
    void drawControls(const Game& game) {
        struct Row { const char* tecla; const char* que; };
        // Una fila con tecla vacia es un titulo de seccion.
        std::vector<Row> filas = {
            { "",        "MODOS" },
            { "J",       "JUGAR TU MISMO" },
            { "M / 1",   "DEMO AUTOMATICA" },
            { "2",       "PATH TRACER (MONTE CARLO)" },
            { "3",       "CINEMATICA INVERSA" },
            { "",        "VISTA" },
            { "T",       "PHONG / TOON (NPR)" },
            { "O",       "AUTO-ORBITA DE CAMARA" },
            { "MOUSE",   "GIRAR ALREDEDOR" },
            { "SCROLL",  "ACERCAR / ALEJAR" },
            { "",        "ACCIONES" },
            { "ESPACIO", spaceAction(game) },
            { "H",       "OCULTAR ESTA AYUDA" },
            { "ESC",     "SALIR" },
        };

        const float esc = 2.0f;       // escala del texto de la tarjeta
        const float capW = 104.0f;    // ancho fijo de la columna de teclas
        const float capH = ov.textHeight(esc) + 12.0f;
        const float pitch = capH + 6.0f, pitchSec = 30.0f;

        // Ancho del panel: se ajusta al texto mas largo.
        float maxDesc = 0.0f;
        for (const Row& r : filas)
            maxDesc = std::max(maxDesc, ov.textWidth(r.que, esc));
        float px = 24.0f, py = 112.0f;
        float pw = 18.0f + capW + 14.0f + maxDesc + 18.0f;

        float ph = 46.0f;             // titulo de la tarjeta
        for (const Row& r : filas) ph += (r.tecla[0] == '\0') ? pitchSec : pitch;
        ph += 12.0f;

        helpRight = px + pw;
        ov.panel(px, py, pw, ph, FONDO, BORDE);
        ov.text(px + 18.0f, py + 14.0f, 2.0f, ACENTO, "CONTROLES");
        ov.rect(px + 18.0f, py + 36.0f, pw - 36.0f, 2.0f, BORDE);

        float y = py + 50.0f;
        for (const Row& r : filas) {
            if (r.tecla[0] == '\0') {                       // titulo de seccion
                ov.text(px + 18.0f, y + 8.0f, 2.0f, ORO, r.que);
                y += pitchSec;
                continue;
            }
            ov.keyCap(px + 18.0f, y, esc, r.tecla, CAPBG, CAPBOR, BLANCO, capW);
            ov.text(px + 18.0f + capW + 14.0f,
                    y + (capH - ov.textHeight(esc)) * 0.5f, esc, BLANCO, r.que);
            y += pitch;
        }
    }

    // Cuando la ayuda esta oculta, queda un recordatorio discreto.
    void drawHint() {
        float y = 112.0f;
        ov.keyCap(24.0f, y, 2.0f, "H", CAPBG, CAPBOR, BLANCO, 0.0f);
        ov.text(24.0f + 44.0f, y + 7.0f, 2.0f, GRIS, "MOSTRAR CONTROLES");
    }

    // ---------- FILA DE GIROS: U D L R F B (solo en modo jugar) ----------
    void drawFaceKeys(const Animator& anim, bool ayudaVisible) {
        struct Cara { const char* tecla; const char* nombre; };
        const Cara caras[6] = {
            { "U", "ARRIBA" }, { "D", "ABAJO" }, { "L", "IZQ" },
            { "R", "DER" },    { "F", "FRENTE" },{ "B", "ATRAS" }
        };
        const float capW = 72.0f, capH = 52.0f, gap = 10.0f;
        float total = 6.0f * capW + 5.0f * gap;

        float pw = total + 50.0f, ph = 148.0f;
        // Se coloca en el hueco que deja la tarjeta de controles.
        float cx = ayudaVisible ? (helpRight + W) * 0.5f : W * 0.5f;
        float px = cx - pw * 0.5f;
        float minX = ayudaVisible ? helpRight + 12.0f : 24.0f;
        if (px < minX)             px = minX;
        if (px + pw > W - 24.0f)   px = W - 24.0f - pw;
        float py = H - ph - 18.0f;

        ov.panel(px, py, pw, ph, FONDO, BORDE);
        ov.textCentered(px + pw * 0.5f, py + 12.0f, 2.0f, ACENTO, "GIRAR UNA CARA");

        // Se apagan las teclas mientras la mezcla inicial esta en curso.
        bool listo = anim.getState() == Animator::State::PLAYING;
        glm::vec4 bg  = listo ? CAPBG  : glm::vec4(0.08f, 0.09f, 0.12f, 0.85f);
        glm::vec4 bor = listo ? CAPBOR : glm::vec4(0.40f, 0.44f, 0.52f, 0.60f);
        glm::vec4 tx  = listo ? BLANCO : GRIS;

        float x = px + 25.0f, y = py + 34.0f;
        for (const Cara& c : caras) {
            ov.rect(x, y, capW, capH, bg);
            ov.frame(x, y, capW, capH, 2.0f, bor);
            ov.textCentered(x + capW * 0.5f, y + 15.0f, 3.0f, tx, c.tecla);
            ov.textCentered(x + capW * 0.5f, y + capH + 8.0f, 2.0f, GRIS, c.nombre);
            x += capW + gap;
        }
        ov.textCentered(px + pw * 0.5f, py + ph - 26.0f, 2.0f, ORO,
                        "SHIFT + TECLA = GIRO INVERSO");
    }

    // ---------- BARRA DE ESTADO (abajo a la izquierda) ----------
    void drawStatusBar(const Game& game, const Animator& anim) {
        char linea[160];
        switch (game.mode) {
        case Game::Mode::PLAY:
            if (anim.getState() == Animator::State::SCRAMBLING)
                snprintf(linea, sizeof(linea), "MEZCLANDO EL CUBO...");
            else
                snprintf(linea, sizeof(linea), "MOVIMIENTOS: %d    TIEMPO: %.1f S",
                         anim.getMoveCount(), anim.getPlayTime());
            break;
        case Game::Mode::PATHTRACER:
            snprintf(linea, sizeof(linea),
                     "MUESTRAS ACUMULADAS: %d    (MAS MUESTRAS = MENOS RUIDO)",
                     game.ptFrame);
            break;
        case Game::Mode::IK:
            snprintf(linea, sizeof(linea),
                     "FABRIK: EL BRAZO PERSIGUE EL OBJETIVO ROJO");
            break;
        default:
            snprintf(linea, sizeof(linea),
                     "LA DEMO MEZCLA Y RESUELVE SOLA - PULSA J PARA JUGAR TU");
            break;
        }

        float esc = 2.0f;
        float bw = ov.textWidth(linea, esc) + 32.0f, bh = 34.0f;
        float bx = 24.0f, by = H - bh - 18.0f;
        ov.panel(bx, by, bw, bh, FONDO, BORDE);
        ov.text(bx + 16.0f, by + (bh - ov.textHeight(esc)) * 0.5f, esc, BLANCO, linea);

        // Record de la sesion, abajo a la derecha.
        if (game.bestMoves > 0) {
            char rec[128];
            snprintf(rec, sizeof(rec), "RECORD: %d MOVS / %.1f S",
                     game.bestMoves, game.bestTime);
            float rw = ov.textWidth(rec, esc) + 32.0f;
            float rx = W - 24.0f - rw;
            ov.panel(rx, by, rw, bh, FONDO, BORDE);
            ov.text(rx + 16.0f, by + (bh - ov.textHeight(esc)) * 0.5f, esc, ORO, rec);
        }
    }

    // ---------- MENSAJES GRANDES (mezclando / resuelto) ----------
    void drawBigMessage(const Game& game, const Animator& anim, float time) {
        if (game.mode == Game::Mode::PATHTRACER || game.mode == Game::Mode::IK) return;

        // Se centra en el espacio libre que deja la tarjeta de controles,
        // asi el mensaje nunca tapa el cubo ni la ayuda.
        float cx = game.showHelp ? (helpRight + W) * 0.5f : W * 0.5f;
        float cy = H * 0.16f;

        if (anim.getState() == Animator::State::CELEBRATING) {
            // Parpadeo suave para que el mensaje resalte sobre la celebracion.
            float pulso = 0.75f + 0.25f * sinf(time * 6.0f);
            glm::vec4 c = ORO; c.a = pulso;
            ov.textCentered(cx, cy, 6.0f, c, "RESUELTO!");
            if (game.mode == Game::Mode::PLAY) {
                char sub[128];
                snprintf(sub, sizeof(sub), "%d MOVIMIENTOS EN %.1f SEGUNDOS",
                         anim.getMoveCount(), anim.getPlayTime());
                ov.textCentered(cx, cy + 56.0f, 2.0f, BLANCO, sub);
                ov.textCentered(cx, cy + 84.0f, 2.0f, ACENTO,
                                "PULSA ESPACIO PARA OTRA PARTIDA");
            }
        } else if (anim.getState() == Animator::State::SCRAMBLING) {
            ov.textCentered(cx, cy, 4.0f, ACENTO, "MEZCLANDO...");
        }
    }
};
