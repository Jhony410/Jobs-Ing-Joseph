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

## 🛠️ Tecnologías y Dependencias

| Tecnología | Uso |
|-----------|-----|
| **C++** | Lenguaje principal |
| **OpenGL 3.3 Core Profile** | API gráfica |
| **GLFW** | Creación de ventana y manejo de input |
| **GLAD** | Carga de funciones de OpenGL |
| **GLSL** | Lenguaje de shaders |
| **stb_image** | Carga de imágenes (ejemplos 4 y 5) |
| **GLM** | Matemáticas para gráficos (ejemplo 5) |

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
```

Los ejemplos siguen un orden pedagógico donde cada uno añade un concepto nuevo sobre lo aprendido anteriormente, construyendo gradualmente una comprensión completa del pipeline de OpenGL moderno.

---

> **Nota:** Estos ejemplos fueron desarrollados como material de estudio para la asignatura de Computación Gráfica. Cada proyecto es autocontenido e incluye todas las dependencias necesarias para compilar y ejecutar.
