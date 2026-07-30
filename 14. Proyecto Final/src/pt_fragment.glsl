#version 330 core
in vec2 vUV;
uniform sampler2D uPrev;   // acumulado de frames anteriores
uniform int  uFrame;       // numero de muestra
uniform vec2 uRes;
out vec4 FragColor;

// ---------- RNG (PCG hash): la base de Monte Carlo ----------
uint seed;
uint pcg() { seed = seed * 747796405u + 2891336453u;
    uint w = ((seed >> ((seed >> 28u) + 4u)) ^ seed) * 277803737u;
    return (w >> 22u) ^ w; }
float rnd() { return float(pcg()) / 4294967296.0; }

// muestreo coseno de la hemiesfera alrededor de N
vec3 cosineHemisphere(vec3 N) {
    float r1 = rnd(), r2 = rnd();
    float phi = 6.2831853 * r1, r = sqrt(r2);
    vec3 T = normalize(abs(N.x) > 0.5 ? cross(N, vec3(0,1,0))
                                      : cross(N, vec3(1,0,0)));
    vec3 B = cross(N, T);
    return normalize(T*cos(phi)*r + B*sin(phi)*r + N*sqrt(1.0-r2));
}

// ---------- Escena: esferas {centro, radio} ----------
struct Sph { vec3 c; float r; vec3 albedo; vec3 emit; };
const int NS = 6;
Sph S[NS];
void buildScene() {
    S[0] = Sph(vec3( 0.0,-100.5,0.0), 100.0, vec3(0.75), vec3(0));           // piso
    S[1] = Sph(vec3(-1.1, 0.0, 0.0), 0.5, vec3(0.9,0.15,0.15), vec3(0));    // roja
    S[2] = Sph(vec3( 0.0, 0.0, 0.0), 0.5, vec3(0.2,0.5,0.95),  vec3(0));    // azul
    S[3] = Sph(vec3( 1.1, 0.0, 0.0), 0.5, vec3(0.2,0.85,0.3),  vec3(0));    // verde
    S[4] = Sph(vec3( 0.0, 2.2, 0.5), 0.7, vec3(0), vec3(14.0,13.0,11.0));   // LUZ
    S[5] = Sph(vec3( 0.4,-0.35,1.0), 0.15, vec3(0.95,0.8,0.2), vec3(0));    // dorada
}

float hitSphere(vec3 ro, vec3 rd, vec3 c, float R) {
    vec3 oc = ro - c;
    float b = dot(oc, rd);
    float disc = b*b - (dot(oc,oc) - R*R);
    if (disc < 0.0) return -1.0;
    float t = -b - sqrt(disc);
    return (t > 0.001) ? t : -1.0;
}

int traceScene(vec3 ro, vec3 rd, out float tMin) {
    int id = -1; tMin = 1e9;
    for (int i = 0; i < NS; i++) {
        float t = hitSphere(ro, rd, S[i].c, S[i].r);
        if (t > 0.0 && t < tMin) { tMin = t; id = i; }
    }
    return id;
}

// ---------- Path tracing: la ecuacion de render via Monte Carlo ----------
vec3 pathTrace(vec3 ro, vec3 rd) {
    vec3 L = vec3(0.0), beta = vec3(1.0);
    for (int b = 0; b < 5; b++) {
        float t; int id = traceScene(ro, rd, t);
        if (id < 0) {                             // cielo
            L += beta * mix(vec3(0.02,0.03,0.06), vec3(0.1,0.15,0.3),
                            rd.y * 0.5 + 0.5);
            break;
        }
        L    += beta * S[id].emit;                // luz emitida
        beta *= S[id].albedo;                     // absorcion
        vec3 P = ro + t * rd;
        vec3 N = normalize(P - S[id].c);
        rd = cosineHemisphere(N);                 // rebote aleatorio
        ro = P + N * 0.001;
    }
    return L;
}

void main() {
    buildScene();
    seed = uint(gl_FragCoord.x) * 1973u + uint(gl_FragCoord.y) * 9277u
         + uint(uFrame) * 26699u;

    // camara fija con jitter sub-pixel (anti-aliasing gratis)
    vec2 jit = vec2(rnd(), rnd()) - 0.5;
    vec2 uv = ((gl_FragCoord.xy + jit) / uRes) * 2.0 - 1.0;
    uv.x *= uRes.x / uRes.y;

    vec3 ro = vec3(0.0, 0.8, 3.2);
    vec3 rd = normalize(vec3(uv.x, uv.y - 0.25, -1.8));

    vec3 c = pathTrace(ro, rd);

    // acumulacion: promedio incremental de muestras Monte Carlo
    // (la textura guarda el color ya codificado; lo decodificamos a lineal)
    vec3 e = texture(uPrev, vUV).rgb;
    vec3 pl = pow(e, vec3(2.2));
    pl = pl / (1.0 - min(pl, vec3(0.999)));          // inverso del tone map
    vec3 acc = (uFrame <= 1) ? c : mix(pl, c, 1.0 / float(uFrame));

    // tone mapping + gamma para mostrar y almacenar
    FragColor = vec4(pow(acc / (acc + 1.0), vec3(1.0/2.2)), 1.0);
}
