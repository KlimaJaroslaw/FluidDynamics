int AIR = 0;
int SOLID = 1;
int FLUID = 2;

struct Particle {
    vec4 color;
    vec3 pos;
    vec3 vel;
    int type;
};

struct GridCell {
    vec3 pos;
    int type;
    vec3 vel;
    float rhs;
    vec3 old_vel;
    float a_diag;
    float a_x;
    float a_y;
    float a_z;
    float pressure_guess;
    float pressure;
    int vel_unknown;
};

struct DebugLine {
    vec4 color;
    vec3 a;
    vec3 b;
};

struct Queue {
    int val;
};

layout(std430, binding=0) restrict buffer ParticleBlock {
    Particle particle[];
};

layout(std430, binding=1) restrict buffer GridBlock {
    GridCell cell[];
};

layout(std430, binding=2) restrict buffer DebugLinesBlock {
    DebugLine debug_lines[];
};
layout(std430, binding=4) restrict buffer QueueBlock {
    Queue queue[];
};

uniform ivec3 grid_dim;
uniform vec3 bounds_min;
uniform vec3 bounds_max;
uniform ivec2 resolution;

ivec3 grid_cell_dim = grid_dim - ivec3(1);
vec3 bounds_size = bounds_max - bounds_min;
vec3 cell_size = bounds_size / vec3(grid_dim - ivec3(1));

const float density = 1; // kg/m^3

bool grid_in_bounds(ivec3 grid_coord) {
    return grid_coord.x >= 0 && grid_coord.y >= 0 && grid_coord.z >= 0 &&
           grid_coord.x < grid_dim.x && grid_coord.y < grid_dim.y && grid_coord.z < grid_dim.z;
}

ivec3 get_grid_coord(vec3 pos, ivec3 half_offset) {
    return ivec3(floor((pos + vec3(half_offset) * (cell_size / 2.0) - bounds_min) / bounds_size * vec3(grid_cell_dim)));
}

vec3 get_world_coord(ivec3 grid_coord, ivec3 half_offset) {
    return bounds_min + vec3(grid_coord) * cell_size + vec3(half_offset) * cell_size * 0.5;
}

int get_grid_index(ivec3 grid_coord) {
    ivec3 clamped_coord = clamp(grid_coord, ivec3(0), grid_dim - ivec3(1));
    return clamped_coord.z * (grid_dim.x * grid_dim.y) + clamped_coord.y * grid_dim.x + clamped_coord.x;
}

ivec3 offset_clamped(ivec3 base_coord, ivec3 dimension_offset) {
    ivec3 max_size = grid_cell_dim;
    if (dimension_offset.x > 0)
        max_size.x = grid_dim.x;
    if (dimension_offset.y > 0)
        max_size.y = grid_dim.y;
    if (dimension_offset.z > 0)
        max_size.z = grid_dim.z;
    return clamp(base_coord + dimension_offset, ivec3(0), max_size - ivec3(1));
}




uniform int container_shape;
uniform float container_radius;
uniform float container_height;

bool pointInContainer_vecSphere(vec3 p, vec3 center, float r) {
    return distance(p, center) <= r + 1e-6;
}

vec3 project_inside_sphere(vec3 p, vec3 center, float r) {
    vec3 dir = p - center;
    float len = length(dir);
    if (len <= 1e-6) {
        // if exactly at center, push to arbitrary point on +X
        return center + vec3(r, 0.0, 0.0);
    }
    return center + normalize(dir) * r;
}

bool pointInContainer_vecCylinder(vec3 p, vec3 center, float r, float h) {
    float half_h = h * 0.5;
    float dy = p.y - center.y;
    if (dy < -half_h || dy > half_h) return false;
    vec2 dxz = (p.xz - center.xz);
    return dot(dxz, dxz) <= r*r + 1e-6;
}

vec3 project_inside_cylinder(vec3 p, vec3 center, float r, float h) {
    float half_h = h * 0.5;
    float y = clamp(p.y, center.y - half_h, center.y + half_h);
    vec2 dxz = p.xz - center.xz;
    float len = length(dxz);
    vec2 projected;
    if (len <= 1e-6) {
        projected = vec2(r, 0.0);
    } else {
        projected = dxz * (r / len);
    }
    return vec3(center.x + projected.x, y, center.z + projected.y);
}

// pointInContainer: przyjmuje world-space point i zwraca true gdy wewnątrz pojemnika
bool pointInContainer(vec3 p) {
    // center of container
    vec3 center = (bounds_min + bounds_max) * 0.5;
    if (container_shape == 0) { // CUBE
        // bounds are already in world coords; cube is entire bounds
        return all(lessThanEqual(abs(p - center), (bounds_max - bounds_min) * 0.5 + vec3(1e-6)));
    } else if (container_shape == 1) { // SPHERE
        return pointInContainer_vecSphere(p, center, container_radius);
    } else { // CYLINDER (container_shape == 2)
        return pointInContainer_vecCylinder(p, center, container_radius, container_height);
    }
}

vec3 project_inside_container(vec3 p) {
    vec3 center = (bounds_min + bounds_max) * 0.5;
    if (container_shape == 0) { // CUBE
        vec3 half_v = (bounds_max - bounds_min) * 0.5;
        return clamp(p, center - half_v, center + half_v);
    } else if (container_shape == 1) { // SPHERE
        return project_inside_sphere(p, center, container_radius);
    } else { // CYLINDER
        return project_inside_cylinder(p, center, container_radius, container_height);
    }
}
