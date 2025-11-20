layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main() {
    ivec3 grid_pos = ivec3(gl_WorkGroupID);
    uint index = get_grid_index(grid_pos);

    // cell center in world coords
    vec3 cell_center = get_world_coord(grid_pos, ivec3(0)) + cell_size * 0.5;

    // set AIR if inside container, SOLID if outside
    if (pointInContainer(cell_center)) {
        cell[index].type = AIR;
    } else {
        cell[index].type = SOLID;
    }

    cell[index].vel = vec3(0);
}
