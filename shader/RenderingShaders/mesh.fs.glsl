out vec4 FragColor;

// Dane przychodzące z Vertex Shadera
in vec3 Normal;    // Normalne (znormalizowane w VS lub tutaj)
in vec3 FragPos;   // Pozycja wierzchołka w świecie 3D

// Pozycja kamery (niezbędna do obliczenia wektora "look")
uniform vec3 viewPos;

// --- TWOJE FUNKCJE (z małą poprawką) ---

vec3 phong_shade(vec3 light_direction, vec3 look, vec3 normal, vec3 ka, vec3 kd, vec3 ks, float shininess) {
    // POPRAWKA: minus przed light_direction
    vec3 R = reflect(-light_direction, normal);

    float diffuse = max(0.0, dot(light_direction, normal));
    float specular = pow(max(0.0, dot(R, look)), shininess);

    return ka + kd * diffuse + ks * specular;
}

vec3 shade(vec3 pos, vec3 look, vec3 normal, vec3 ka, vec3 kd, vec3 ks, float shininess) {
    // Twoja logika jest super: Ambient (ka) dodajemy raz, a do świateł przekazujemy vec3(0) jako ambient.
    // Dodałem tylko 0.0 do floatów dla bezpieczeństwa typów.

    return ka +
    phong_shade(normalize(vec3(-0.5, 0.8, 0.0) - pos), look, normal, vec3(0.0), kd, ks, shininess) +
    phong_shade(normalize(vec3( 0.5, 0.8, 0.0) - pos), look, normal, vec3(0.0), kd, ks, shininess);
}

// --- MAIN ---

void main()
{
    // 1. Przygotowanie wektorów
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos); // To jest Twój wektor "look"

    // 2. Definicja materiału (skoro nie ma .mtl)
    // Szary kot:
    vec3 ka = vec3(0.1, 0.1, 0.1); // Ambient (bardzo ciemny szary)
    vec3 kd = vec3(0.6, 0.6, 0.6); // Diffuse (właściwy kolor kota - szary)
    vec3 ks = vec3(1.0, 1.0, 1.0); // Specular (kolor błysku światła - biały)
    float shininess = 32.0;        // Połyskliwość (im więcej, tym mniejsza i ostrzejsza plamka światła)

    // 3. Wywołanie Twojej funkcji
    vec3 result = shade(FragPos, viewDir, norm, ka, kd, ks, shininess);

    FragColor = vec4(result, 1.0);
}