layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

uniform float dt;
uniform vec3 look;
uniform vec3 eye;
uniform vec3 mouse_pos;
uniform vec3 mouse_vel;

const float mouse_range = 0.25;

bool ray_sphere_isect(vec3 r0, vec3 rd, vec3 s0, float sr) {
    // - r0: ray origin
    // - rd: normalized ray direction
    // - s0: sphere center
    // - sr: sphere radius
    // - Returns distance from r0 to first intersecion with sphere,
    //   or -1.0 if no intersection.
    float a = dot(rd, rd);
    vec3 s0_r0 = r0 - s0;
    float b = 2.0 * dot(rd, s0_r0);
    float c = dot(s0_r0, s0_r0) - (sr * sr);
    if (b*b - 4.0*a*c < 0.0) {
        return false;
    }
    return true;
}

void main() {
    uint index = gl_GlobalInvocationID.x; // Lepiej używać GlobalInvocationID dla 1D
    if (index >= particle.length()) return; // Zabezpieczenie

    // 1. Kopia starej pozycji i prędkości
    vec3 pos = particle[index].pos;
    vec3 vel = particle[index].vel;

    // 2. Adwekcja (Forward Euler)
    // Nowa pozycja "kandydat"
    pos += vel * dt;

    if(particle[index].type == 0) { // Tylko dla FLUID

        // 3. Jitter (przesunięcie, żeby uniknąć regularnych wzorów)
        const float jitter = 0.005; // Wartość przykładowa
        // Upewnij się, że hash3 zwraca vec3 w zakresie -1 do 1 lub 0 do 1 i odpowiednio przeskaluj
        // Tutaj zakładam, że hash zwraca 0..1, więc odejmuję 0.5
        pos += (hash3(floatBitsToInt(pos)) - 0.5) * jitter;

        // 4. Clamp do granic domeny (Bounds check)
        // Bardzo ważne: zrób to PRZED pobraniem indeksu siatki,
        // żeby nie wyjść poza tablicę cell[]
        vec3 epsilon = vec3(cell_size * 0.5); // Pół komórki marginesu jest bezpieczne
        pos = clamp(pos, bounds_min + epsilon, bounds_max - epsilon);

        // 5. Sprawdzenie kolizji z SDF (Solid Interaction)
        ivec3 grid_coord = get_grid_coord(pos, ivec3(1,1,1)); // Używamy NOWEJ pozycji
        uint cell_idx = get_grid_index(grid_coord);

        // Pobieramy dystans i gradient z komórki, w której znalazła się cząsteczka
        float dist = cell[cell_idx].dist;
        vec3 grad = cell[cell_idx].grad;

        // Jeśli dystans < 0, cząsteczka jest wewnątrz przeszkody
        if(dist < 0.0) {
            // A. Korekta Pozycji:
            // Wypychamy cząsteczkę na powierzchnię w kierunku gradientu.
            // Ponieważ dist jest ujemne, odejmujemy (dist * grad), co daje (plus wartość * grad).
            pos -= dist * grad;

            // B. Korekta Prędkości (Warunek brzegowy):
            // Musimy usunąć tę część prędkości, która pcha cząsteczkę w ścianę.
            // v_new = v_old - (v_old dot N) * N
            float normal_vel = dot(vel, grad);

            // Sprawdzamy, czy prędkość faktycznie jest skierowana w ścianę (normal_vel < 0)
            if(normal_vel < 0.0) {
                vel -= 1.5*normal_vel * grad;

                // Opcjonalnie: Tarcie (Friction)
                 vel *= 0.9;
            }
        }

        // 6. Interakcja z myszką (opcjonalne)
        if (ray_sphere_isect(mouse_pos, normalize(mouse_pos - eye), pos, mouse_range)) {
            vel += mouse_vel;
        }
    }

    // 7. Zapisz wyniki do bufora
    particle[index].pos = pos;
    particle[index].vel = vel;
}
