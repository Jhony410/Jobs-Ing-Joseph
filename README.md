# 🎮 Ejemplos de OpenGL — Computación Gráfica

Colección de ejemplos progresivos de **OpenGL moderno (Core Profile 3.3)** desarrollados y estudiados en la clase de **Computación Gráfica** durante el semestre 2026-I.

Cada carpeta es un proyecto independiente en **C++** que demuestra un concepto fundamental del pipeline gráfico programable, desde el triángulo más básico hasta texturas con movimiento en tiempo real.

---

## 📂 Estructura del Repositorio

| # | Carpeta | Concepto principal |
|---|---------|-------------------|
| 1 | `1. Triangulo coloreado` | VAO/VBO, vertex & fragment shaders, interpolación de colores |
| 2 | `2. EBO Cuadrado` | Element Buffer Object (EBO) e indexado de vértices |
| 3 | `3. EBO Casa` | Composición de figuras con múltiples triángulos indexados |
| 4 | `4. Delfin` | Geometría compleja (meshes), uniforms, animación y blending |
| 4 | `4. Texturas` | Carga de imágenes con `stb_image`, mapeo UV y texturas 2D |
| 5 | `5. Texturas y Movimiento` | Matrices de transformación con GLM, delta time e input de teclado |
| 7 | `7. Mundo 3D` | Cubo 3D texturizado con matrices MVP, Z-Buffer y proyección perspectiva |
| 8 | `8. Mundo 3D interactivo` | Cámara orbital con mouse, zoom con teclado y delta time |
| 9 | `9. Multiples Objetos` | Renderizado de múltiples cubos con un solo VAO/VBO y bucle de instancias |
| 10 | `10. Iluminacion` | Modelo de iluminación Phong (ambiental + difusa + especular) con normales |
| 11 | `11. Materiales` | Struct `Material` en GLSL con propiedades por objeto (esmeralda, oro, rubí…) |
| 11.1 | `11.1. Materiales - Esferas` | Generación procedural de esferas con materiales contrastantes |
| 12 | `12. Mapas de Iluminación, Texturas` | Mapas difuso y especular con `sampler2D`, multi-textura y scroll zoom |

---

## 🔍 Descripción de cada Ejemplo

### 1. Triángulo Coloreado
Punto de partida para OpenGL moderno. Se define un triángulo con **posiciones y colores por vértice** usando un `VBO` y un `VAO`. Los shaders (`vertex.glsl` / `fragment.glsl`) se leen desde archivos externos. El fragment shader interpola los colores automáticamente entre los tres vértices, produciendo un degradado RGB.

**Conceptos clave:**
- Vertex Buffer Object (VBO) y Vertex Array Object (VAO)
- Compilación y enlazado de shaders GLSL
- Atributos de vértice con `glVertexAttribPointer`
- Renderizado con `glDrawArrays`

---

### 2. EBO Cuadrado
Introduce el **Element Buffer Object (EBO)** para reutilizar vértices y dibujar un cuadrado con solo 4 vértices en lugar de 6. Se definen índices que indican qué vértices componen cada triángulo.

**Conceptos clave:**
- Element Buffer Object (EBO)
- Indexado de vértices con `glDrawElements`
- Optimización de geometría al evitar vértices duplicados

---

### 3. EBO Casa
Extiende el uso de EBO para crear una **figura compuesta**: una casa formada por un rectángulo (pared) y un triángulo (techo), todo con 5 vértices y 3 triángulos indexados. Cada vértice tiene un color distinto.

**Conceptos clave:**
- Composición de figuras geométricas complejas con EBOs
- Diseño de mallas con múltiples triángulos
- Planificación de geometría mediante índices

---

### 4. Delfín
Ejemplo avanzado que dibuja una **escena completa de un delfín en un atardecer** sobre el océano. Incluye:
- Geometría del delfín con 19 vértices y 18 triángulos
- Fondo con degradado de cielo
- Sol con anillos concéntricos (usando `GL_TRIANGLE_FAN`)
- Océano con olas animadas usando funciones seno
- Contorno del delfín con `GL_LINES`

**Conceptos clave:**
- Meshes complejos con struct `Mesh`
- Uniforms (`uOffset`, `uScale`, `uSolidColor`)
- Generación procedural de círculos y elipses
- Animación en el render loop con `glfwGetTime()`
- Alpha blending (`GL_BLEND`)
- Modos de dibujo: `GL_TRIANGLES`, `GL_TRIANGLE_FAN`, `GL_LINE_STRIP`, `GL_LINES`

---

### 4. Texturas
Introduce el **mapeo de texturas** sobre geometría. Se carga una imagen (`logo.png`) usando la librería `stb_image` y se aplica sobre un cuadrado mediante coordenadas UV.

**Conceptos clave:**
- Carga de imágenes con `stb_image.h`
- Creación y configuración de texturas OpenGL (`glGenTextures`, `glTexParameteri`)
- Coordenadas de textura (UV)
- Generación de mipmaps con `glGenerateMipmap`
- Atributos de vértice con stride de 8 (posición + color + UV)

---

### 5. Texturas y Movimiento
Combina texturas con **transformaciones en tiempo real**. El usuario puede mover el cuadrado texturizado con las flechas del teclado. Utiliza la librería **GLM** para crear matrices de transformación y el concepto de **delta time** para movimiento fluido e independiente del framerate.

**Conceptos clave:**
- Biblioteca GLM (`glm::mat4`, `glm::translate`)
- Matrices de transformación enviadas como uniforms a la GPU
- Delta time para movimiento consistente
- Procesamiento de input de teclado con GLFW
- Uniform `mat4` con `glUniformMatrix4fv`

---

### 7. Mundo 3D
Da el salto de 2D a **3D real** dibujando un **cubo texturizado** que rota automáticamente. Introduce las tres matrices fundamentales del 3D: **Modelo** (posiciona y rota el objeto), **Vista** (simula la cámara) y **Proyección** (aplica perspectiva). Activa el **Z-Buffer** (`GL_DEPTH_TEST`) para que las caras del cubo se oculten correctamente según la profundidad.

**Conceptos clave:**
- Geometría 3D: cubo de 36 vértices (6 caras × 2 triángulos × 3 vértices)
- Matrices MVP (Model-View-Projection) con GLM
- `glm::rotate`, `glm::translate`, `glm::perspective`
- Z-Buffer con `glEnable(GL_DEPTH_TEST)` y `GL_DEPTH_BUFFER_BIT`
- Rotación automática basada en `glfwGetTime()`
- Vertex shader con multiplicación `projection * view * model * vec4(aPos, 1.0)`

---

### 8. Mundo 3D Interactivo
Extiende el cubo 3D agregando **interactividad completa**. El usuario puede orbitar la cámara alrededor del cubo con el **mouse**, hacer **zoom** con las teclas `W`/`S`, y rotar el modelo con `A`/`D`. Implementa una **cámara orbital** usando coordenadas esféricas y la función `glm::lookAt`. Utiliza **delta time** para que el movimiento sea fluido e independiente del framerate.

**Conceptos clave:**
- Cámara orbital con coordenadas esféricas (yaw, pitch, radius)
- `glm::lookAt` para construir la matriz de vista
- Callback de mouse con `glfwSetCursorPosCallback`
- Captura del cursor con `GLFW_CURSOR_DISABLED`
- Delta time (`deltaTime`) para movimiento consistente
- Control de zoom y rotación del modelo por teclado
- Límites de pitch (−89° a 89°) para evitar gimbal lock

---

### 9. Múltiples Objetos
Demuestra cómo renderizar **10 cubos texturizados** en distintas posiciones del espacio 3D usando un **único VAO/VBO** compartido. En cada frame se recorre un arreglo de posiciones y se aplica una **matriz modelo diferente** por cubo (traslación + rotación individual). Las matrices de vista y proyección se envían una sola vez por frame, mientras que la matriz modelo se actualiza en cada iteración del bucle.

**Conceptos clave:**
- Instanciación manual con bucle `for` y arreglo de posiciones `cubePositions[]`
- Una sola llamada a `glBindVertexArray` por frame, múltiples `glDrawArrays`
- Rotación individual por objeto: ángulo base `20° × i` + rotación global
- Reutilización de geometría y textura entre instancias
- Separación de uniforms por frecuencia (view/projection una vez, model por objeto)

---

### 10. Iluminación
Introduce el **modelo de iluminación Phong** completo sobre los 10 cubos. Cada vértice ahora incluye un **vector normal** además de su posición. El fragment shader calcula tres componentes de luz: **ambiental** (luz base constante), **difusa** (depende del ángulo entre la normal y la dirección de la luz) y **especular** (reflejo brillante que depende de la posición del observador). La fuente de luz **orbita automáticamente** alrededor de la escena.

**Conceptos clave:**
- Vectores normales por vértice (stride de 6 floats: posición + normal)
- Componente ambiental: `ambientStrength * lightColor`
- Componente difusa: `max(dot(normal, lightDir), 0.0)` (Ley de Lambert)
- Componente especular: `pow(max(dot(viewDir, reflectDir), 0.0), 32)` (Phong)
- Matriz normal: `mat3(transpose(inverse(model)))` para transformar normales correctamente
- `FragPos` en espacio mundo para cálculos de iluminación en el fragment shader
- Uniforms: `lightPos`, `viewPos`, `lightColor`, `objectColor`
- Luz en movimiento con funciones seno/coseno sobre `glfwGetTime()`

---

### 11. Materiales
Reemplaza el color fijo del objeto (`objectColor`) por un **struct `Material`** en GLSL que define propiedades de superficie individuales: `ambient`, `diffuse`, `specular` y `shininess`. Cada uno de los 10 cubos recibe un **material diferente** (esmeralda, oro, plata, rubí, turquesa, perla, obsidiana, cromo, latón y plástico rojo), lo que produce respuestas de iluminación visualmente distintas bajo la misma fuente de luz.

**Conceptos clave:**
- Struct `Material` en GLSL con propiedades `ambient`, `diffuse`, `specular`, `shininess`
- Struct `MaterialData` en C++ como espejo del struct GLSL
- Arreglo de 10 materiales predefinidos con valores realistas (tabla de materiales OpenGL)
- Envío de campos struct como uniforms: `material.ambient`, `material.diffuse`, etc.
- `shininess * 128.0` para mapear el rango del exponente especular
- Cada componente de luz ahora se multiplica por la propiedad correspondiente del material

---

### 11.1. Materiales — Esferas
Variación del ejemplo anterior que reemplaza los cubos por **esferas generadas proceduralmente**. La función `crearEsfera()` genera una malla de triángulos a partir de coordenadas esféricas (anillos × sectores), con normales calculadas automáticamente. Las 10 esferas se disponen en un **círculo** y presentan 5 materiales contrastantes (goma mate, cromo/espejo, plástico azul, oro metálico y arcilla roja), cada uno duplicado para comparación.

**Conceptos clave:**
- Generación procedural de esferas con `rings` y `sectors` (60×60 = alta resolución)
- Coordenadas esféricas → cartesianas para posiciones y normales
- Indexado de triángulos a partir de cuadriláteros (2 triángulos por quad)
- Uso de `std::vector<float>` para geometría dinámica
- Disposición circular de objetos: `cos(36° × i) × radio`, `sin(36° × i) × radio`
- Animación de flotación con `sin(time + i)` por esfera
- `glm::scale` para escalar las esferas

---

### 12. Mapas de Iluminación (Texturas)
Reemplaza los colores sólidos del material por **mapas de textura**: un **mapa difuso** (`difuso.png`) que define el color base de la superficie, y un **mapa especular** (`especular.png`) en blanco y negro que controla **dónde brilla** el objeto. El vertex shader ahora pasa coordenadas UV al fragment shader, que las usa para muestrear ambas texturas con `sampler2D`. También añade **zoom con la rueda del ratón** mediante `glfwSetScrollCallback`.

**Conceptos clave:**
- Struct `Material` con `sampler2D diffuse` y `sampler2D specular` en lugar de `vec3`
- Multi-textura: `GL_TEXTURE0` para el mapa difuso, `GL_TEXTURE1` para el especular
- `glActiveTexture` + `glBindTexture` para activar unidades de textura
- Función reutilizable `cargarTextura()` con detección automática de canales (RGB/RGBA)
- Vértices con stride de 8 floats: posición (3) + normal (3) + UV (2)
- Zoom con rueda del ratón: `glfwSetScrollCallback` con límites de cercanía/lejanía
- El mapa especular en B/N actúa como máscara: zonas negras no producen brillo

---

## 🛠️ Tecnologías y Dependencias

| Tecnología | Uso |
|-----------|-----|
| **C++** | Lenguaje principal |
| **OpenGL 3.3 Core Profile** | API gráfica |
| **GLFW** | Creación de ventana y manejo de input |
| **GLAD** | Carga de funciones de OpenGL |
| **GLSL** | Lenguaje de shaders |
| **stb_image** | Carga de imágenes (ejemplos 4, 5, 7, 8, 9 y 12) |
| **GLM** | Matemáticas para gráficos (ejemplos 5, 7–12) |

---

## 🚀 Compilación y Ejecución

Cada carpeta incluye un archivo `compilar.bat` que compila y ejecuta el proyecto automáticamente.

### Requisitos
- **MinGW** (g++) instalado y en el `PATH`
- Sistema operativo **Windows**

### Pasos
```bash
# 1. Navegar a la carpeta del ejemplo deseado
cd "1. Triangulo coloreado"

# 2. Ejecutar el script de compilación
compilar.bat
```

El comando de compilación utilizado es:
```bash
g++ src/main.cpp src/glad.c -o programa.exe -I include -L lib -lglfw3 -lgdi32 -lopengl32
```

---

## 📖 Estructura de cada Proyecto

```
📁 Ejemplo/
├── 📁 include/          # Headers (GLAD, GLFW, GLM)
├── 📁 lib/              # Librerías estáticas (.a)
├── 📁 src/
│   ├── main.cpp         # Código fuente principal
│   ├── glad.c           # Implementación de GLAD
│   ├── vertex.glsl      # Vertex shader
│   └── fragment.glsl    # Fragment shader
├── compilar.bat         # Script de compilación
└── programa.exe         # Ejecutable generado
```

---

## 📌 Progresión de Aprendizaje

```
Triángulo ──► EBO Cuadrado ──► EBO Casa ──► Delfín (escena completa)
                                               │
                                          Texturas ──► Texturas + Movimiento
                                                              │
                                                        Mundo 3D ──► Mundo 3D Interactivo
                                                                            │
                                                                   Múltiples Objetos
                                                                            │
                                                                      Iluminación
                                                                            │
                                                              Materiales ──► Materiales (Esferas)
                                                                            │
                                                              Mapas de Iluminación
```

Los ejemplos siguen un orden pedagógico donde cada uno añade un concepto nuevo sobre lo aprendido anteriormente, construyendo gradualmente una comprensión completa del pipeline de OpenGL moderno.

---

> **Nota:** Estos ejemplos fueron desarrollados como material de estudio para la asignatura de Computación Gráfica. Cada proyecto es autocontenido e incluye todas las dependencias necesarias para compilar y ejecutar.
