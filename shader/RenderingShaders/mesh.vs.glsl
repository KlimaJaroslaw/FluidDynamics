layout (location = 0) in vec3 aPos;    // Vertex.x, y, z
layout (location = 1) in vec3 aNormal; // Vertex.nx, ny, nz
layout (location = 2) in vec2 aTexCoord; // Vertex.u, v (opcjonalne, na razie nieużywane)

// Wyjścia do Fragment Shadera
out vec3 FragPos;  // Pozycja wierzchołka w świecie 3D
out vec3 Normal;   // Wektor normalny

// Zmienne globalne (Uniformy) przesyłane z C++
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // 1. Obliczenie pozycji wierzchołka w przestrzeni świata
    // Mnożymy tylko przez macierz modelu
    FragPos = vec3(model * vec4(aPos, 1.0));

    // 2. Obliczenie poprawnego wektora normalnego
    // Używamy "Normal Matrix" (transpozycja odwrotności), żeby skalowanie modelu nie zepsuło oświetlenia
    Normal = mat3(transpose(inverse(model))) * aNormal;

    // 3. Finalna pozycja na ekranie (Clip Space)
    gl_Position = projection * view * vec4(FragPos, 1.0);
}