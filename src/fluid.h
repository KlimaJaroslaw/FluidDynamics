//
// Created by tk2 on 11/20/25.
//

#ifndef GL_PIC_FLUID_FLUID_H
#define GL_PIC_FLUID_FLUID_H


#pragma once
#include <vector>
#include <stdexcept>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/vec_swizzle.hpp>
#include <Eigen/Core>
#include <Eigen/Sparse>
#include "GridCell.hpp"
#include "Particle.hpp"
#include "DebugLine.hpp"
#include "P2GTransfer.hpp"
#include "SSFBufferElement.hpp"
#include "SSFRenderTexture.hpp"
#include "Quad.hpp"
#include "Queue.hpp"
#include "util.hpp"
#include "gfx/object.hpp"
#include "gfx/program.hpp"
#include "gfx/rendertexture.hpp"
#include "shapes/Ramp.hpp"

struct Fluid {
    const int num_circle_vertices = 8; // circle detail for particle rendering

    const int particle_density = 2;
    const int grid_size = 32;
    int num_mesh_vertices;
    const glm::ivec3 grid_dimensions{grid_size + 1, grid_size + 1, grid_size + 1};
    const glm::ivec3 grid_cell_dimensions{grid_size, grid_size, grid_size};
    const glm::vec3 bounds_min{-1, -1, -1};
    const glm::vec3 bounds_max{1, 1, 1};
    const glm::vec3 bounds_size = bounds_max - bounds_min;
    const glm::vec3 cell_size = bounds_size / glm::vec3(grid_cell_dimensions);
    const glm::vec3 gravity{0, -9.8, 0};
    glm::vec3 world_mouse_pos{0, -0.9, 0};
    glm::vec3 world_mouse_vel{0, 0, 0};
    glm::vec3 look{0, 0, 1};
    glm::vec3 eye{0, 0, 0};
    glm::ivec2 resolution{0, 0};
    float pic_flip_blend = 0.9;

    gfx::Buffer viewport_rect{GL_ARRAY_BUFFER};
    gfx::Buffer particle_ssbo{GL_SHADER_STORAGE_BUFFER}; // particle data storage
    gfx::Buffer grid_ssbo{GL_SHADER_STORAGE_BUFFER}; // grid data storage
    gfx::Buffer queue_ssbo{GL_SHADER_STORAGE_BUFFER}; //queue for incoming particles
    gfx::Buffer transfer_ssbo{GL_SHADER_STORAGE_BUFFER}; // p2g transfer storage buffer
    gfx::Buffer circle_verts{GL_ARRAY_BUFFER};
    gfx::Buffer debug_lines_ssbo{GL_SHADER_STORAGE_BUFFER};
    gfx::VAO vao;
    gfx::VAO grid_vao;
    gfx::VAO debug_lines_vao; // used for drawing colored lines for debugging
    gfx::VAO screen_quad_vao; // fullscreen quad vertices


    gfx::Program reset_grid_program; // clear grid state
    gfx::Program p2g_accumulate_program; // accumulate new grid velocities from particles
    gfx::Program p2g_apply_program; // copy new grid velocities to grid data
    gfx::Program particle_advect_program; // compute shader to operate on particles SSBO
    gfx::Program body_forces_program; // compute shader to apply body forces on grid
    gfx::Program extrapolate_program; // extrapolate grid velocities by one cell
    gfx::Program set_vel_known_program;
    gfx::Program setup_grid_project_program; // compute A and RHS of pressure equation
    gfx::Program jacobi_iterate_program; // single jacobi iteration to solve for pressure gradient
    gfx::Program pressure_to_guess_program; // copy pressure to pressure_guess for pressure solve
    gfx::Program pressure_update_program; // update velocities from pressure gradient
    gfx::Program grid_to_particle_program; // transfer grid velocities to particles
    gfx::Program put_on_queue_program; //Deactivates drained particles and puts them on a queue
    gfx::Program read_queue_program; //Activates particles from source if they are on a queue
    gfx::Program sum_up_queue_program; //sums up an array in log(n) time

    gfx::Program program; // program for particle rendering
    gfx::Program grid_program;
    gfx::Program debug_lines_program;

    // screen space fluid rendering
    gfx::Program ssf_spheres_program; // SSF sphere rendering
    gfx::Program ssf_smooth_program; // SSF smoothing
    gfx::Program ssf_shade_program; // SSF shading

    gfx::RenderTexture scene_texture; // stores rendered things that are not fluid
    SSFRenderTexture ssf_a_texture; // stores sphere color/depth data, and world space position
    SSFRenderTexture ssf_b_texture; // it's double buffered for read/write

    Quad quad;
    Fluid() {}

    void init();
    inline int idx(int gx, int gy, int gz);
    void addNeighbors(std::vector<GridCell>& grid);
    void init_ssbos();

    void resize(glm::uint w, glm::uint h);
    glm::ivec3 get_grid_coord(const glm::vec3& pos, const glm::ivec3& half_offset = glm::ivec3(0, 0, 0));
    glm::vec3 get_world_coord(const glm::ivec3& grid_coord, const glm::ivec3& half_offset = glm::ivec3(0, 0, 0));
    bool grid_in_bounds(const glm::ivec3& grid_coord);
    int get_grid_index(const glm::ivec3& grid_coord);

    void set_common_uniforms(gfx::Program& program);
    void reset_grid();
    void particle_to_grid();
    void extrapolate();
    void apply_body_forces(float dt);
    void setup_grid_project(float dt);
    void pressure_solve();
    void pressure_update(float dt);
    void grid_to_particle();
    void particle_advect(float dt);

    void put_on_queue();
    void read_queue();
    void ssbo_barrier();

    void step();

    void draw_particles(const glm::mat4& projection, const glm::mat4& view, const glm::vec4& viewport);
    void draw_mesh(const glm::mat4& projection, const glm::mat4& view, const glm::vec4& viewport);
    void draw_particles_ssf(const gfx::RenderTexture& scene_texture, const glm::mat4& projection, const glm::mat4& view, const glm::vec4& viewport);
    void draw_grid(const glm::mat4& projection, const glm::mat4& view, int display_mode);
    void draw_debug_lines(const glm::mat4& projection, const glm::mat4& view);
};

#endif //GL_PIC_FLUID_FLUID_H