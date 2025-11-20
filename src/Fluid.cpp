#include "fluid.h"


void Fluid::init() {
    init_ssbos();

    // graphics initialization
    // circle vertices (for triangle fan)
    std::vector<glm::vec2> circle;
    for (int i = 0; i < num_circle_vertices; ++i) {
        const float f = static_cast<float>(i) / num_circle_vertices * glm::pi<float>() * 2.0;
        circle.emplace_back(glm::vec2(glm::sin(f), glm::cos(f)));
    }
    circle_verts.set_data(circle);
    // Here you can give arguments to the rendering shaders
    vao.bind_attrib(circle_verts, 2, GL_FLOAT)
       .bind_attrib(particle_ssbo, offsetof(Particle, pos), sizeof(Particle), 3, GL_FLOAT, gfx::INSTANCED)
       .bind_attrib(particle_ssbo, offsetof(Particle, vel), sizeof(Particle), 3, GL_FLOAT, gfx::INSTANCED)
       .bind_attrib(particle_ssbo, offsetof(Particle, color), sizeof(Particle), 4, GL_FLOAT, gfx::INSTANCED)
    .bind_attrib(particle_ssbo,offsetof(Particle,type), sizeof(Particle),1,GL_INT,gfx::INSTANCED);

    grid_vao.bind_attrib(grid_ssbo, offsetof(GridCell, pos), sizeof(GridCell), 3, GL_FLOAT, gfx::NOT_INSTANCED)
       .bind_attrib(grid_ssbo, offsetof(GridCell, vel), sizeof(GridCell), 3, GL_FLOAT, gfx::NOT_INSTANCED)
       .bind_attrib(grid_ssbo, offsetof(GridCell, type), sizeof(GridCell), 1, GL_INT, gfx::NOT_INSTANCED)
       .bind_attrib(grid_ssbo, offsetof(GridCell, rhs), sizeof(GridCell), 1, GL_FLOAT, gfx::NOT_INSTANCED)
       .bind_attrib(grid_ssbo, offsetof(GridCell, a_diag), sizeof(GridCell), 4, GL_FLOAT, gfx::NOT_INSTANCED)
       .bind_attrib(grid_ssbo, offsetof(GridCell, pressure), sizeof(GridCell), 1, GL_FLOAT, gfx::NOT_INSTANCED)
       .bind_attrib(grid_ssbo, offsetof(GridCell, vel_unknown), sizeof(GridCell), 1, GL_INT, gfx::NOT_INSTANCED)
        .bind_attrib(grid_ssbo,offsetof(GridCell, nType),sizeof(GridCell), 1, GL_INT, gfx::NOT_INSTANCED);

    debug_lines_vao.bind_attrib(debug_lines_ssbo, offsetof(DebugLine, a), sizeof(DebugLine), 3, GL_FLOAT, gfx::NOT_INSTANCED)
        .bind_attrib(debug_lines_ssbo, offsetof(DebugLine, b), sizeof(DebugLine), 3, GL_FLOAT, gfx::NOT_INSTANCED)
        .bind_attrib(debug_lines_ssbo, offsetof(DebugLine, color), sizeof(DebugLine), 4, GL_FLOAT, gfx::NOT_INSTANCED);

    put_on_queue_program.compute({"ComputeShaders/common.glsl","ComputeShaders/put_on_queue.cs.glsl"}).compile();
    read_queue_program.compute({"ComputeShaders/common.glsl","ComputeShaders/rand.glsl","ComputeShaders/read_queue.cs.glsl"}).compile();
    sum_up_queue_program.compute({"ComputeShaders/common.glsl","ComputeShaders/sum_up_queue.cs.glsl"}).compile();
    reset_grid_program.compute({"ComputeShaders/common.glsl", "ComputeShaders/reset_grid.cs.glsl"}).compile();
    p2g_accumulate_program.compute({"ComputeShaders/atomic.glsl", "ComputeShaders/common.glsl", "ComputeShaders/p2g_common.glsl", "ComputeShaders/p2g_accumulate.cs.glsl"}).compile();
    p2g_apply_program.compute({"ComputeShaders/atomic.glsl", "ComputeShaders/common.glsl", "ComputeShaders/p2g_common.glsl", "ComputeShaders/p2g_apply.cs.glsl"}).compile();
    grid_to_particle_program.compute({"ComputeShaders/common.glsl", "ComputeShaders/grid_to_particle.cs.glsl"}).compile();
    extrapolate_program.compute({"ComputeShaders/common.glsl", "ComputeShaders/extrapolate.cs.glsl"}).compile();
    set_vel_known_program.compute({"ComputeShaders/common.glsl", "ComputeShaders/set_vel_known.cs.glsl"}).compile();
    body_forces_program.compute({"ComputeShaders/common.glsl", "ComputeShaders/enforce_boundary.cs.glsl", "ComputeShaders/body_forces.cs.glsl"}).compile();
    setup_grid_project_program.compute({"ComputeShaders/common.glsl","ComputeShaders/rand.glsl", "ComputeShaders/setup_project.cs.glsl", "ComputeShaders/compute_divergence.cs.glsl", "ComputeShaders/build_a.cs.glsl"}).compile();
    jacobi_iterate_program.compute({"ComputeShaders/common.glsl", "ComputeShaders/jacobi_iterate.cs.glsl"}).compile();
    pressure_to_guess_program.compute({"ComputeShaders/common.glsl", "ComputeShaders/pressure_to_guess.cs.glsl"}).compile();
    pressure_update_program.compute({"ComputeShaders/common.glsl", "ComputeShaders/pressure_update.cs.glsl"}).compile();
    particle_advect_program.compute({"ComputeShaders/common.glsl", "ComputeShaders/rand.glsl", "ComputeShaders/particle_advect.cs.glsl"}).compile();



    program.vertex({"RenderingShaders/particles.vs.glsl"}).fragment({"RenderingShaders/lighting.glsl", "RenderingShaders/particles.fs.glsl"}).compile();
    grid_program.vertex({"ComputeShaders/common.glsl", "RenderingShaders/grid.vs.glsl"}).geometry({"ComputeShaders/common.glsl", "RenderingShaders/grid.gs.glsl"}).fragment({"RenderingShaders/grid.fs.glsl"}).compile();
    debug_lines_program.vertex({"RenderingShaders/debug_lines.vs.glsl"}).geometry({"RenderingShaders/debug_lines.gs.glsl"}).fragment({"RenderingShaders/debug_lines.fs.glsl"}).compile();

    ssf_spheres_program.vertex({"RenderingShaders/particles.vs.glsl"}).fragment({"ComputeShaders/common.glsl", "RenderingShaders/ssf_spheres.fs.glsl"}).compile();
    ssf_smooth_program.vertex({"RenderingShaders/screen_quad.vs.glsl"}).fragment({"RenderingShaders/ssf_smooth.fs.glsl"}).compile();
    ssf_shade_program.vertex({"RenderingShaders/screen_quad.vs.glsl"}).fragment({"RenderingShaders/lighting.glsl", "RenderingShaders/ssf_shade.fs.glsl"}).compile();
}

inline int Fluid::idx(int gx, int gy, int gz)
{
    return gx + gy * grid_cell_dimensions.x + gz * grid_cell_dimensions.x * grid_cell_dimensions.y;
}
void Fluid::addNeighbors(std::vector<GridCell>& grid)
{
    int nt[grid.size()];
    for (int i=0;i<grid.size();i++){nt[i]=0;}
    for (int gz = 0; gz < grid_dimensions.z; ++gz)
    {
        for (int gy = 0; gy < grid_dimensions.y; ++gy)
        {
            for (int gx = 0; gx < grid_dimensions.x; ++gx)
            {
                int i = idx(gx, gy, gz);
                if (grid[i].type == GRID_SOLID)
                {
                    if (gx > 0 && grid[idx(gx - 1, gy, gz)].type != GRID_SOLID)
                        nt[idx(gx - 1, gy, gz)] = 1;


                    if (gx < grid_dimensions.x - 1 && grid[idx(gx + 1, gy, gz)].type != GRID_SOLID)
                        nt[idx(gx + 1, gy, gz)] = 2;


                    if (gy > 0 && grid[idx(gx, gy - 1, gz)].type != GRID_SOLID)
                        nt[idx(gx, gy-1, gz)] = 3;


                    if (gy < grid_dimensions.y - 1 && grid[idx(gx, gy + 1, gz)].type != GRID_SOLID)
                        nt[idx(gx, gy+1, gz)] = 4;


                    if (gz > 0 && grid[idx(gx, gy, gz - 1)].type != GRID_SOLID)
                        nt[idx(gx, gy, gz-1)] = 5;


                    if (gz < grid_dimensions.z - 1 && grid[idx(gx, gy, gz + 1)].type != GRID_SOLID)
                        nt[idx(gx, gy, gz+1)] = 6;
                }
            }
        }
    }
    for (int i=0;i<grid.size();i++){grid[i].nType=nt[i];}
}

void Fluid::init_ssbos() {
    std::vector<GridCell> initial_grid;
    std::vector<Particle> initial_particles;
    std::vector<P2GTransfer> initial_transfer;
    std::vector<Queue> initial_queue;
    for (int gz = 0; gz < grid_dimensions.z; ++gz) {
        for (int gy = 0; gy < grid_dimensions.y; ++gy) {
            for (int gx = 0; gx < grid_dimensions.x; ++gx) {
                const glm::ivec3 gpos{gx, gy, gz};
                const glm::vec3 cell_pos = get_world_coord(gpos);

                initial_transfer.emplace_back(P2GTransfer());
                // It justs places particles when x is less then half
                // TODO: add a way to make any fluid shape, also to make constant fluid flow
                const glm::ivec3& d = grid_cell_dimensions;
                if (gx < d.x*0.5) {
                    initial_grid.emplace_back(GridCell{
                        cell_pos,
                        glm::vec3(0),
                        GRID_FLUID
                    });

                    if (gx < grid_cell_dimensions.x && gy < grid_cell_dimensions.y && gz < grid_cell_dimensions.z) {
                        for (int i = 0; i < particle_density; ++i) {
                            const glm::vec3 particle_pos = glm::linearRand(cell_pos, cell_pos + cell_size);
                            initial_particles.emplace_back(Particle{
                                particle_pos,
                                glm::vec3(0),
                                glm::vec4(0.32,0.57,0.79,1.0),
                                0
                            });
                            initial_queue.emplace_back(Queue());
                        }
                    }
                }
                else {
                    initial_grid.emplace_back(GridCell{
                        cell_pos,
                        glm::vec3(0),
                        GRID_AIR
                    });
                }
            }
        }
    }
    // std::unique_ptr<IShape> shape = std::make_unique<Ramp>();
    // auto points1 = shape->to_grid(1,1,1,24);
    // for (auto point : points1)
    // {
    //     glm::vec3 pos = point-glm::vec3(0.5,0.5,0.5);
    //     glm::ivec3 gCoord = get_grid_coord(pos);
    //     uint ind = get_grid_index(gCoord);
    //     std::cout << "X: " << gCoord.x << "Y: " << gCoord.y << "Z: " << gCoord.z << std::endl;
    //
    //     initial_grid[ind]=GridCell{
    //                     pos,
    //                     glm::vec3(0),
    //                     GRID_SOLID};
    // }
    addNeighbors(initial_grid);
    particle_ssbo.bind_base(0).set_data(initial_particles, GL_DYNAMIC_COPY);
    grid_ssbo.bind_base(1).set_data(initial_grid, GL_DYNAMIC_COPY);
    std::cerr << "Cell count: " << initial_grid.size() << std::endl;
    std::cerr << "Particle count: " << initial_particles.size() << std::endl;

    std::vector<DebugLine> debug_lines;
    debug_lines.push_back(DebugLine({0, 0, 0}, {0.1, 0, 0}, {1, 0, 0, 1})); // x axis
    debug_lines.push_back(DebugLine({0, 0, 0}, {0, 0.1, 0}, {0, 1, 0, 1})); // y axis
    debug_lines.push_back(DebugLine({0, 0, 0}, {0, 0, 0.1}, {0, 0, 1, 1})); // z axis
    debug_lines_ssbo.bind_base(2).set_data(debug_lines);

    transfer_ssbo.bind_base(3).set_data(initial_transfer, GL_DYNAMIC_COPY);
    queue_ssbo.bind_base(4).set_data(initial_queue,GL_DYNAMIC_COPY);
    std::cout << "Size of debug lines buffer " << debug_lines_ssbo.length() << " (" << debug_lines_ssbo.size() << " bytes)" << std::endl;
}


void Fluid::resize(glm::uint w, glm::uint h) {
    scene_texture.set_texture_size(w, h);
    ssf_a_texture.set_texture_size(w, h);
    ssf_b_texture.set_texture_size(w, h);
    resolution.x = w;
    resolution.y = h;
}

glm::ivec3 Fluid::get_grid_coord(const glm::vec3& pos, const glm::ivec3& half_offset) {
    return glm::floor((pos + glm::vec3(half_offset) * (cell_size / 2.f) - bounds_min) / bounds_size * glm::vec3(grid_cell_dimensions));
}

glm::vec3 Fluid::get_world_coord(const glm::ivec3& grid_coord, const glm::ivec3& half_offset) {
    return bounds_min + glm::vec3(grid_coord) * cell_size + glm::vec3(half_offset) * cell_size * 0.5f;
}

bool Fluid::grid_in_bounds(const glm::ivec3& grid_coord) {
    return (grid_coord.x >= 0 && grid_coord.y >= 0 && grid_coord.z >= 0 &&
            grid_coord.x < grid_dimensions.x && grid_coord.y < grid_dimensions.y && grid_coord.z < grid_dimensions.z);
}

int Fluid::get_grid_index(const glm::ivec3& grid_coord) {
    const glm::ivec3 clamped_coord = glm::clamp(grid_coord, glm::ivec3(0), grid_dimensions - glm::ivec3(1));
    return clamped_coord.z * grid_dimensions.x * grid_dimensions.y + clamped_coord.y * grid_dimensions.x + clamped_coord.x;
}

void Fluid::set_common_uniforms(gfx::Program& program) {
    glUniform3fv(program.uniform_loc("bounds_min"), 1, glm::value_ptr(bounds_min));
    glUniform3fv(program.uniform_loc("bounds_max"), 1, glm::value_ptr(bounds_max));
    glUniform3iv(program.uniform_loc("grid_dim"), 1, glm::value_ptr(grid_dimensions));
    glUniform2iv(program.uniform_loc("resolution"), 1, glm::value_ptr(resolution));
}

void Fluid::reset_grid() {
    ssbo_barrier();
    reset_grid_program.use();
    set_common_uniforms(reset_grid_program);
    reset_grid_program.validate();
    glDispatchCompute(grid_dimensions.x, grid_dimensions.y, grid_dimensions.z);
    reset_grid_program.disuse();
}

void Fluid::particle_to_grid() {
    constexpr static int group_size = 1024;
    reset_grid();

    p2g_accumulate_program.use();
    set_common_uniforms(p2g_accumulate_program);

    // accumulate
    for (int i = 0; i < particle_ssbo.length() / group_size + 1; ++i) {
        glUniform1i(p2g_accumulate_program.uniform_loc("start_index"), i * group_size);
        p2g_accumulate_program.validate();
        ssbo_barrier();
        glDispatchCompute(1, 1, 1);
    }

    // copy transfer accumulators to grid velocities
    ssbo_barrier();
    p2g_apply_program.use();
    set_common_uniforms(p2g_apply_program);
    glDispatchCompute(grid_dimensions.x, grid_dimensions.y, grid_dimensions.z);
    p2g_apply_program.disuse();
}

void Fluid::extrapolate() {
    extrapolate_program.use();
    glUniform3fv(extrapolate_program.uniform_loc("bounds_min"), 1, glm::value_ptr(bounds_min));
    glUniform3fv(extrapolate_program.uniform_loc("bounds_max"), 1, glm::value_ptr(bounds_max));
    glUniform3iv(extrapolate_program.uniform_loc("grid_dim"), 1, glm::value_ptr(grid_dimensions));
    extrapolate_program.validate();

    set_vel_known_program.use();
    glUniform3fv(set_vel_known_program.uniform_loc("bounds_min"), 1, glm::value_ptr(bounds_min));
    glUniform3fv(set_vel_known_program.uniform_loc("bounds_max"), 1, glm::value_ptr(bounds_max));
    glUniform3iv(set_vel_known_program.uniform_loc("grid_dim"), 1, glm::value_ptr(grid_dimensions));
    set_vel_known_program.validate();

    for (int i = 0; i < glm::compMax(grid_dimensions) * 2; ++i) {
        ssbo_barrier();
        extrapolate_program.use();
        glDispatchCompute(grid_dimensions.x, grid_dimensions.y, grid_dimensions.z);

        ssbo_barrier();
        set_vel_known_program.use();
        glDispatchCompute(grid_dimensions.x, grid_dimensions.y, grid_dimensions.z);
    }
}

void Fluid::apply_body_forces(float dt) {
    // also enforces boundary condition
    ssbo_barrier();
    body_forces_program.use();
    const glm::vec3 body_force = gravity; // TODO: other forces?
    glUniform1f(body_forces_program.uniform_loc("dt"), dt);
    glUniform3fv(body_forces_program.uniform_loc("bounds_min"), 1, glm::value_ptr(bounds_min));
    glUniform3fv(body_forces_program.uniform_loc("bounds_max"), 1, glm::value_ptr(bounds_max));
    glUniform3iv(body_forces_program.uniform_loc("grid_dim"), 1, glm::value_ptr(grid_dimensions));
    glUniform3fv(body_forces_program.uniform_loc("body_force"), 1, glm::value_ptr(body_force));
    body_forces_program.validate();
    glDispatchCompute(grid_dimensions.x, grid_dimensions.y, grid_dimensions.z);
    body_forces_program.disuse();
}

void Fluid::setup_grid_project(float dt) {

    ssbo_barrier();
    setup_grid_project_program.use();
    glUniform1f(setup_grid_project_program.uniform_loc("dt"), dt);
    glUniform3fv(setup_grid_project_program.uniform_loc("bounds_min"), 1, glm::value_ptr(bounds_min));
    glUniform3fv(setup_grid_project_program.uniform_loc("bounds_max"), 1, glm::value_ptr(bounds_max));
    glUniform3iv(setup_grid_project_program.uniform_loc("grid_dim"), 1, glm::value_ptr(grid_dimensions));
    setup_grid_project_program.validate();
    glDispatchCompute(grid_dimensions.x, grid_dimensions.y, grid_dimensions.z);
    setup_grid_project_program.disuse();
}

void Fluid::pressure_solve() {
    const int iters = 40;

    jacobi_iterate_program.use();
    set_common_uniforms(jacobi_iterate_program);
    jacobi_iterate_program.validate();

    pressure_to_guess_program.use();
    set_common_uniforms(pressure_to_guess_program);
    pressure_to_guess_program.validate();

    for (int i = 0; i < iters; ++i) {
        ssbo_barrier();
        jacobi_iterate_program.use();
        glDispatchCompute(grid_dimensions.x, grid_dimensions.y, grid_dimensions.z);

        ssbo_barrier();
        pressure_to_guess_program.use();
        glDispatchCompute(grid_dimensions.x, grid_dimensions.y, grid_dimensions.z);
    }
}

void Fluid::pressure_update(float dt) {
    ssbo_barrier();
    pressure_update_program.use();
    glUniform1f(pressure_update_program.uniform_loc("dt"), dt);
    glUniform3fv(pressure_update_program.uniform_loc("bounds_min"), 1, glm::value_ptr(bounds_min));
    glUniform3fv(pressure_update_program.uniform_loc("bounds_max"), 1, glm::value_ptr(bounds_max));
    glUniform3iv(pressure_update_program.uniform_loc("grid_dim"), 1, glm::value_ptr(grid_dimensions));
    pressure_update_program.validate();
    glDispatchCompute(grid_dimensions.x, grid_dimensions.y, grid_dimensions.z);
    pressure_update_program.disuse();
}

void Fluid::grid_to_particle() {
    ssbo_barrier();
    grid_to_particle_program.use();
    glUniform3fv(grid_to_particle_program.uniform_loc("bounds_min"), 1, glm::value_ptr(bounds_min));
    glUniform3fv(grid_to_particle_program.uniform_loc("bounds_max"), 1, glm::value_ptr(bounds_max));
    glUniform3iv(grid_to_particle_program.uniform_loc("grid_dim"), 1, glm::value_ptr(grid_dimensions));
    glUniform1f(grid_to_particle_program.uniform_loc("pic_flip_blend"), pic_flip_blend);
    grid_to_particle_program.validate();
    glDispatchCompute(particle_ssbo.length(), 1, 1);
    grid_to_particle_program.disuse();
}

void Fluid::particle_advect(float dt) {
    ssbo_barrier();
    particle_advect_program.use();
    glUniform1f(particle_advect_program.uniform_loc("dt"), dt);
    glUniform3fv(particle_advect_program.uniform_loc("bounds_min"), 1, glm::value_ptr(bounds_min));
    glUniform3fv(particle_advect_program.uniform_loc("bounds_max"), 1, glm::value_ptr(bounds_max));
    glUniform3iv(particle_advect_program.uniform_loc("grid_dim"), 1, glm::value_ptr(grid_dimensions));
    glUniform3fv(particle_advect_program.uniform_loc("eye"), 1, glm::value_ptr(eye));
    glUniform3fv(particle_advect_program.uniform_loc("mouse_pos"), 1, glm::value_ptr(world_mouse_pos));
    glUniform3fv(particle_advect_program.uniform_loc("mouse_vel"), 1, glm::value_ptr(world_mouse_vel));
    glDispatchCompute(particle_ssbo.length(), 1, 1);
    particle_advect_program.disuse();
}

void Fluid::put_on_queue()
{
    ssbo_barrier();
    put_on_queue_program.use();
    glDispatchCompute(particle_ssbo.length(),1,1);
    put_on_queue_program.disuse();
}

void Fluid::read_queue()
{
    //Why do i have to do this to sum up an array????
    for (int i=0;i<std::log2(particle_ssbo.length());i++)
    {
        ssbo_barrier();
        sum_up_queue_program.use();
        glUniform1i(sum_up_queue_program.uniform_loc("power"),i);
        glUniform1i(sum_up_queue_program.uniform_loc("limit"),50);
        glDispatchCompute(particle_ssbo.length(),1,1);
        sum_up_queue_program.disuse();
    }
    ssbo_barrier();
    read_queue_program.use();
    glUniform3fv(read_queue_program.uniform_loc("bounds_min"), 1, glm::value_ptr(bounds_min));
    glUniform3fv(read_queue_program.uniform_loc("bounds_max"), 1, glm::value_ptr(bounds_max));
    glUniform1i(read_queue_program.uniform_loc("limit"),50);
    glDispatchCompute(particle_ssbo.length(),1,1);
    read_queue_program.disuse();
}

void Fluid::ssbo_barrier() {
    // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glMemoryBarrier.xhtml
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void Fluid::step() {
    const float dt = 0.01;
    particle_to_grid();
    // extrapolate();
    apply_body_forces(dt);
    setup_grid_project(dt);
    pressure_solve();
    pressure_update(dt);
    grid_to_particle();
    particle_advect(dt);
}

void Fluid::draw_particles(const glm::mat4& projection, const glm::mat4& view, const glm::vec4& viewport) {
    program.use();
    glUniformMatrix4fv(program.uniform_loc("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(program.uniform_loc("view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniform4fv(program.uniform_loc("viewport"), 1, glm::value_ptr(viewport));
    glUniform3fv(program.uniform_loc("look"), 1, glm::value_ptr(look));
    vao.bind();
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, num_circle_vertices, particle_ssbo.length());
    vao.unbind();
    program.disuse();
}

void Fluid::draw_particles_ssf(const gfx::RenderTexture& scene_texture, const glm::mat4& projection, const glm::mat4& view, const glm::vec4& viewport) {
        constexpr static GLenum ssf_draw_buffers[]{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};

        // render spheres and position data
        ssf_spheres_program.use();
            set_common_uniforms(ssf_spheres_program);
            glUniformMatrix4fv(ssf_spheres_program.uniform_loc("projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(ssf_spheres_program.uniform_loc("view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniform4fv(ssf_spheres_program.uniform_loc("viewport"), 1, glm::value_ptr(viewport));
            glUniform3fv(ssf_spheres_program.uniform_loc("look"), 1, glm::value_ptr(look));

            vao.bind();
            ssf_a_texture.bind_framebuffer();

            // clear buffers
            glDrawBuffers(2, ssf_draw_buffers);
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // color pass
            glEnable(GL_BLEND);
            glDisable(GL_DEPTH_TEST);
            glBlendFunc(GL_ONE, GL_ONE);
            glUniform1i(ssf_spheres_program.uniform_loc("pass"), 0);
            constexpr static GLenum first_pass_buffers[]{GL_COLOR_ATTACHMENT0, GL_NONE};
            glDrawBuffers(2, first_pass_buffers);
            glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, num_circle_vertices, particle_ssbo.length());

            // sphere position pass
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
            glUniform1i(ssf_spheres_program.uniform_loc("pass"), 1);
            constexpr static GLenum second_pass_buffers[]{GL_NONE, GL_COLOR_ATTACHMENT1};
            glDrawBuffers(2, second_pass_buffers);
            glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, num_circle_vertices, particle_ssbo.length());

            ssf_a_texture.unbind_framebuffer();
            vao.unbind();
        ssf_spheres_program.disuse();

        // smooth sphere depths, do color thing
        glDisable(GL_BLEND);
        ssf_smooth_program.use();
            glUniform2iv(ssf_smooth_program.uniform_loc("resolution"), 1, glm::value_ptr(resolution));
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ssf_a_texture.color1_id);
            ssf_b_texture.bind_framebuffer();
            glDrawBuffers(2, ssf_draw_buffers);
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            quad.draw();
            ssf_b_texture.unbind_framebuffer();
            glBindTexture(GL_TEXTURE_2D, 0);
        ssf_smooth_program.disuse();

        // shade fluid
        // glDisable(GL_DEPTH_TEST);
        ssf_shade_program.use();
            glUniform2iv(ssf_shade_program.uniform_loc("resolution"), 1, glm::value_ptr(resolution));
            glUniformMatrix4fv(ssf_shade_program.uniform_loc("projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(ssf_shade_program.uniform_loc("inv_view"), 1, GL_FALSE, glm::value_ptr(glm::inverse(view)));
            glUniform3fv(ssf_shade_program.uniform_loc("look"), 1, glm::value_ptr(look));
            glUniform3fv(ssf_shade_program.uniform_loc("eye"), 1, glm::value_ptr(eye));
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ssf_a_texture.color_id);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, ssf_a_texture.depth_id);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, ssf_b_texture.color1_id); // sphere position data
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, scene_texture.color_id);
            quad.draw();
            glBindTexture(GL_TEXTURE_2D, 0);
        ssf_shade_program.disuse();
    }

void Fluid::draw_grid(const glm::mat4& projection, const glm::mat4& view, int display_mode) {
    grid_program.use();
    glUniform3fv(grid_program.uniform_loc("bounds_min"), 1, glm::value_ptr(bounds_min));
    glUniform3fv(grid_program.uniform_loc("bounds_max"), 1, glm::value_ptr(bounds_max));
    glUniform3iv(grid_program.uniform_loc("grid_dim"), 1, glm::value_ptr(grid_dimensions));
    glUniformMatrix4fv(grid_program.uniform_loc("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(grid_program.uniform_loc("view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniform1i(grid_program.uniform_loc("display_mode"), display_mode);
    grid_vao.bind();
    glPointSize(16.0);
    glDrawArrays(GL_POINTS, 0, grid_ssbo.length());
    grid_vao.unbind();
    grid_program.disuse();
}

void Fluid::draw_debug_lines(const glm::mat4& projection, const glm::mat4& view) {
    debug_lines_program.use();
    glUniformMatrix4fv(debug_lines_program.uniform_loc("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(debug_lines_program.uniform_loc("view"), 1, GL_FALSE, glm::value_ptr(view));
    debug_lines_vao.bind();
    glLineWidth(4.0);
    glDrawArrays(GL_POINTS, 0, debug_lines_ssbo.length());
    debug_lines_vao.unbind();
    debug_lines_program.disuse();
}