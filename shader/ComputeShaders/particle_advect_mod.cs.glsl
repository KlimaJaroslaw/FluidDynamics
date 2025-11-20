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
    // - Returns whether intersection exists
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
    uint index = gl_WorkGroupID.x;

    // explicit Euler integrate position
    particle[index].pos += particle[index].vel * dt;

    // only process active particles (type == 0)
    if (particle[index].type == 0) {
        // jitter particle positions to prevent perfect stacking
        const float jitter = 0.005;
        particle[index].pos += hash3(floatBitsToInt(particle[index].pos)) * jitter - 0.5 * jitter;

        // small epsilon for numerical safety
        vec3 eps = vec3(0.00001);

        // if particle left the container, project it back and correct velocity
        if (!pointInContainer(particle[index].pos)) {
            // project to container surface / interior
            vec3 proj = project_inside_container(particle[index].pos);

            // compute correction normal
            vec3 n = particle[index].pos - proj;
            float nlen = length(n);

            // container center (assume axis-aligned bounds)
            vec3 center = (bounds_min + bounds_max) * 0.5;

            if (nlen < 1e-6) {
                // degenerate case: fallback normal
                if (container_shape == 1) { // sphere
                    n = normalize(particle[index].pos - center);
                } else if (container_shape == 2) { // cylinder
                    // radial in XZ
                    vec2 dxz = particle[index].pos.xz - center.xz;
                    float l = length(dxz);
                    if (l < 1e-6) {
                        n = vec3(0.0, 1.0, 0.0);
                    } else {
                        n = normalize(vec3(dxz.x, 0.0, dxz.y));
                    }
                } else { // cube
                    // push to nearest axis
                    vec3 half_v = (bounds_max - bounds_min) * 0.5;
                    vec3 rel = particle[index].pos - center;
                    vec3 absrel = abs(rel);
                    if (absrel.x >= absrel.y && absrel.x >= absrel.z) {
                        n = vec3(sign(rel.x), 0.0, 0.0);
                    } else if (absrel.y >= absrel.x && absrel.y >= absrel.z) {
                        n = vec3(0.0, sign(rel.y), 0.0);
                    } else {
                        n = vec3(0.0, 0.0, sign(rel.z));
                    }
                }
            } else {
                n = normalize(n);
            }

            // push slightly inside so it won't re-collide next frame
            const float inside_eps = 1e-5;
            particle[index].pos = proj + n * inside_eps;

            // velocity correction: remove outward normal component
            float vn = dot(particle[index].vel, n);
            if (vn > 0.0) {
                const float restitution = 0.0; // set >0 for bounce
                particle[index].vel -= (1.0 + restitution) * vn * n;
            }

            // slight tangential damping to prevent sliding instabilities
            const float tangential_damping = 0.95;
            vec3 vnorm = dot(particle[index].vel, n) * n;
            vec3 vtan = particle[index].vel - vnorm;
            particle[index].vel = vnorm + vtan * tangential_damping;
        }

        // mouse interaction
        bool hit = ray_sphere_isect(mouse_pos, normalize(mouse_pos - eye), particle[index].pos, mouse_range);
        if (hit) {
            particle[index].vel += mouse_vel;
        }

        // clamp to world bounds as final safety net (very small epsilon)
        particle[index].pos = clamp(particle[index].pos, bounds_min + eps, bounds_max - eps);
    }
}
