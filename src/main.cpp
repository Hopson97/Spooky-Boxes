#include <array>
#include <numbers>

#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Window/Event.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include "GUI.h"
#include "Graphics/Camera.h"
#include "Graphics/DebugRenderer.h"
#include "Graphics/GBuffer.h"
#include "Graphics/Lights.h"
#include "Graphics/Mesh.h"
#include "Graphics/Model.h"
#include "Graphics/OpenGL/Framebuffer.h"
#include "Graphics/OpenGL/GLDebugEnable.h"
#include "Graphics/OpenGL/GLResource.h"
#include "Graphics/OpenGL/Shader.h"
#include "Graphics/OpenGL/Texture.h"
#include "Graphics/OpenGL/VertexArray.h"
#include "Physics/PhysicsSystem.h"
#include "Utils/HeightMap.h"
#include "Utils/Maths.h"
#include "Utils/Util.h"

namespace
{
    struct Material
    {
        Texture2D colour_texture;
        Texture2D specular_texture;

        Material(const std::filesystem::path& colour_texture_path,
                 const std::filesystem::path& specular_texture_path)
        {
            colour_texture.load_from_file(colour_texture_path, 8, true, false);
            specular_texture.load_from_file(specular_texture_path, 8, true, false);
        }

        void bind()
        {
            colour_texture.bind(0);
            specular_texture.bind(1);
        }
    };

    template <int Ticks>
    class TimeStep
    {
      public:
        template <typename F>
        void update(F f)
        {
            sf::Time time = timer_.getElapsedTime();
            sf::Time elapsed = time - last_time_;
            last_time_ = time;
            lag_ += elapsed;
            while (lag_ >= timePerUpdate_)
            {
                lag_ -= timePerUpdate_;
                f(dt_.restart());
            }
        }

      private:
        const sf::Time timePerUpdate_ = sf::seconds(1.f / Ticks);
        sf::Clock timer_;
        sf::Clock dt_;
        sf::Time last_time_ = sf::Time::Zero;
        sf::Time lag_ = sf::Time::Zero;
    };

    glm::vec3 get_keyboard_input(const Transform& transform, bool flying)
    {
        // Keyboard Input
        glm::vec3 move{0.0f};
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        {
            move += forward_vector(transform.rotation);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            move += backward_vector(transform.rotation);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            move += left_vector(transform.rotation);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            move += right_vector(transform.rotation);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
        {
            move *= 10.0f;
        }

        return move;
    }

    void get_mouse_move_input(Transform& transform, const sf::Window& window)
    {
        auto& r = transform.rotation;
        static auto last_mouse = sf::Mouse::getPosition(window);
        auto change = sf::Mouse::getPosition(window) - last_mouse;
        r.x -= static_cast<float>(change.y * 0.35);
        r.y += static_cast<float>(change.x * 0.35);
        sf::Mouse::setPosition({(int)window.getSize().x / 2, (int)window.getSize().y / 2}, window);
        last_mouse = sf::Mouse::getPosition(window);

        r.x = glm::clamp(r.x, -89.9f, 89.9f);
        if (r.y >= 360.0f)
        {
            r.y = 0.0f;
        }
        if (r.y < 0.0f)
        {
            r.y = 359.9f;
        }
    }

    struct SSBOMatrix
    {
        glm::mat4 projection_matrix;
        glm::mat4 view_matrix;
    };
} // namespace

int main()
{
    sf::ContextSettings context_settings;
    context_settings.depthBits = 24;
    context_settings.stencilBits = 8;
    context_settings.antialiasingLevel = 4;
    context_settings.majorVersion = 4;
    context_settings.minorVersion = 5;
    context_settings.attributeFlags = sf::ContextSettings::Core;

    sf::Window window({1600, 900}, "g", sf::Style::Default, context_settings);
    window.setVerticalSyncEnabled(true);
    bool mouse_locked = false;

    Settings settings;

    window.setActive(true);

    if (!gladLoadGL())
    {
        std::cerr << "Failed to init OpenGL - Is OpenGL linked correctly?\n";
        return -1;
    }
    glViewport(0, 0, window.getSize().x, window.getSize().y);
    init_opengl_debugging();

    if (!GUI::init(&window))
    {
        return -1;
    }
    srand(static_cast<unsigned>(time(nullptr)));

    // -----------------------------------------------------------
    // ==== Create the Meshes + OpenGL vertex array + GBuffer ====
    // -----------------------------------------------------------
    auto billboard_vertex_array = generate_quad_mesh(1.0f, 2.0f);

    HeightMap height_map{128};
    // auto height_map = HeightMap::from_image("assets/heightmaps/test2.png");
    height_map.set_base_height();
    {
        TerrainGenerationOptions options;
        options.amplitude = 100.0f;
        options.roughness = 0.6f;
        options.octaves = 7;
        options.seed = rand() % 50000;
        std::cout << "Seed: " << options.seed << "\n";

        height_map.generate_terrain(options);
        height_map.set_base_height();
    }
    auto terrain_mesh = generate_terrain_mesh(height_map);
    auto light_vertex_mesh = generate_cube_mesh({0.2f, 0.2f, 0.2f}, false);
    auto box_vertex_mesh = generate_cube_mesh({1.0f, 1.0f, 1.0f}, false);

    auto wall_vertex_mesh = generate_cube_mesh({50.0f, 15.0f, 0.2f}, true);

    Model model;
    model.load_from_file("assets/models/House/House.obj");

    GBuffer gbuffer(window.getSize().x, window.getSize().y);

    // ------------------------------------
    // ==== Create the OpenGL Textures ====
    // ------------------------------------
    Material person_material("assets/textures/person.png", "assets/textures/person_specular.png");
    Material grass_material("assets/textures/grass_03.png", "assets/textures/grass_specular.png");

    Material create_material("assets/textures/crate.png", "assets/textures/crate_specular.png");

    // ---------------------------------------
    // ==== Create the OpenGL Framebuffer ====
    // ---------------------------------------
    Framebuffer fbo(window.getSize().x, window.getSize().y);
    fbo.attach_colour(TextureFormat::RGB8).attach_renderbuffer();
    if (!fbo.is_complete())
    {
        return -1;
    }

    // --------------------------------------------------
    // ==== Create empty VBO for rendering to window ====
    // --------------------------------------------------
    GLuint fbo_vbo;
    glCreateVertexArrays(1, &fbo_vbo);

    // ----------------------
    // ==== Load shaders ====
    // ----------------------
    Shader scene_shader;
    // if (!scene_shader.load_from_file("assets/shaders/ScreenVertex.glsl",
    //                                  "assets/shaders/SceneFragmentDeferred.glsl"))
    //{
    //     return -1;
    // }
    if (!scene_shader.load_from_file("assets/shaders/SceneVertex.glsl",
                                     "assets/shaders/SceneFragment.glsl"))
    {
        return -1;
    }

    Shader gbuffer_shader;
    if (!gbuffer_shader.load_from_file("assets/shaders/GBufferVertex.glsl",
                                       "assets/shaders/GBufferFragment.glsl"))
    {
        return -1;
    }

    Shader fbo_shader;
    if (!fbo_shader.load_from_file("assets/shaders/ScreenVertex.glsl",
                                   "assets/shaders/ScreenFragment.glsl"))
    {
        return -1;
    }

    // -----------------------------------
    // ==== Entity Transform Creation ====
    // -----------------------------------
    Transform terrain_transform;
    Transform light_transform;
    Transform model_transform;
    model_transform.position = {50, height_map.get_height(50, 50), 50};
    model_transform.scale = {2, 2, 2};

    std::array<PointLight, 5> point_lights;
    for (int i = 0; i < 5; i++)
    {
        PointLight p = settings.lights.point_light;
        float x = static_cast<float>(rand() % (height_map.size - 2)) + 1;
        float z = static_cast<float>(rand() % (height_map.size - 2)) + 1;
        p.position = {x, height_map.get_height(x, z), z, 0.0f};
        point_lights[i] = p;
    }

    std::vector<Transform> people_transforms;
    for (int i = 0; i < 50; i++)
    {
        float x = static_cast<float>(rand() % (height_map.size - 2)) + 1;
        float z = static_cast<float>(rand() % (height_map.size - 2)) + 1;

        people_transforms.push_back({{x, height_map.get_height(x, z), z}, {0.0f, 0.0, 0}});
    }

    light_transform.position = {20.0f, 5.0f, 20.0f};

    // -----------------------------------
    // ==== Camera Creation ====
    // -----------------------------------
    PerspectiveCamera camera(window.getSize().x, window.getSize().y, 75.0f);
    // camera.transform.position = {80.0f, 1.0f, 35.0f};
    camera.transform.rotation = {0.0f, 100, 0.0f};
    camera.transform.position = {15, height_map.max_height(), 15};

    // ----------------------------
    // ==== Load sound effects ====
    // ----------------------------
    // Load walking sounds
    sf::SoundBuffer walk0;
    walk0.loadFromFile("assets/sounds/sfx_step_grass_l.ogg");

    sf::SoundBuffer walk1;
    walk1.loadFromFile("assets/sounds/sfx_step_grass_r.ogg");

    std::size_t sound_idx = 0;
    std::array<sf::Sound, 2> walk_sounds;
    walk_sounds[0].setBuffer(walk0);
    walk_sounds[1].setBuffer(walk1);

    auto create_looping_bg =
        [](sf::Music& background_sfx, const std::string path, float volume, float offset)
    {
        background_sfx.openFromFile(path);
        background_sfx.setLoop(true);
        background_sfx.setVolume(volume);
        background_sfx.setPlayingOffset(sf::seconds(offset));
        background_sfx.play();
    };

    // Load ambient night sounds
    sf::Music ambient_night1;
    sf::Music ambient_night2;
    sf::Music spookysphere;
    create_looping_bg(ambient_night1, "assets/sounds/crickets.ogg", 25.0f, 0.0f);
    create_looping_bg(ambient_night2, "assets/sounds/crickets.ogg", 10.0f, 2.0f);
    create_looping_bg(spookysphere, "assets/sounds/Atmosphere_003(Loop).wav", 10.0f, 0.0f);

    // ------------------------------
    // ==== Bullet3D Experiments ====
    // ------------------------------
    // Contains the setup for memory and collisions
    btDefaultCollisionConfiguration collision_config;

    btCollisionDispatcher collision_dispatcher(&collision_config);

    // Collision dispather is <todo>
    std::unique_ptr<btBroadphaseInterface> overlapping_pair_cache =
        std::make_unique<btDbvtBroadphase>();

    // Default constraint solver
    btSequentialImpulseConstraintSolver solver;

    // The world??
    btDiscreteDynamicsWorld dynamics_world(&collision_dispatcher, overlapping_pair_cache.get(),
                                           &solver, &collision_config);

    dynamics_world.setGravity({0, -10, 0});

    // Keep track of created collision shapes so they can be released at exit
    // btAlignedObjectArray<std::unique_ptr<btCollisionShape>> collision_shapes;
    std::vector<PhysicsObject> physics_objects;

    // -------------------------------------------------------
    // ==== Bullet3D Experiments: Create the ground plane ====
    // -------------------------------------------------------

    // The triangle mesh must be kept alive so created in the outer scope
    btTriangleMesh terrain_collision_mesh;
    {
        PhysicsObject& ground = physics_objects.emplace_back();

        // Create the collision mesh
        auto& is = terrain_mesh.indices;
        auto& vs = terrain_mesh.vertices;
        for (int i = 0; i < (int)terrain_mesh.indices.size(); i += 3)
        {
            auto v1 = to_btvec3(vs[is[i]].position);
            auto v2 = to_btvec3(vs[is[i + 1]].position);
            auto v3 = to_btvec3(vs[is[i + 2]].position);
            terrain_collision_mesh.addTriangle(v1, v2, v3);
        }
        ground.collision_shape =
            std::make_unique<btBvhTriangleMeshShape>(&terrain_collision_mesh, true, true);

        btScalar mass = 0.0f;
        btVector3 ground_shape_local_inertia(0, 0, 0);

        btTransform ground_transform;
        ground_transform.setIdentity();
        ground_transform.setOrigin({0, 0, 0});

        // Create the rigid body for the ground
        ground.motion_state = std::make_unique<btDefaultMotionState>(ground_transform);

        btRigidBody::btRigidBodyConstructionInfo ground_rb_info(mass, ground.motion_state.get(),
                                                                ground.collision_shape.get(),
                                                                ground_shape_local_inertia);
        ground_rb_info.m_friction = 1.25f;
        ground.body = std::make_unique<btRigidBody>(ground_rb_info);
        dynamics_world.addRigidBody(ground.body.get());
    }

    // ----------------------------------------------------
    // ==== Bullet3D Experiments: Create a static mesh ====
    // ----------------------------------------------------
    std::vector<std::unique_ptr<btTriangleMesh>> model_collision_meshes;

    for (auto& model_mesh : model.get_meshes())
    {
        auto& mesh = model_mesh.mesh;

        auto& collision_mesh =
            model_collision_meshes.emplace_back(std::make_unique<btTriangleMesh>());
        PhysicsObject& mesh_object = physics_objects.emplace_back();

        auto& is = mesh.indices;
        auto& vs = mesh.vertices;
        for (int i = 0; i < (int)mesh.indices.size(); i += 3)
        {
            auto v1 = to_btvec3(vs[is[i]].position);
            auto v2 = to_btvec3(vs[is[i + 1]].position);
            auto v3 = to_btvec3(vs[is[i + 2]].position);
            collision_mesh->addTriangle(v1, v2, v3);
        }
        collision_mesh->setScaling(to_btvec3(model_transform.scale));

        mesh_object.collision_shape =
            std::make_unique<btBvhTriangleMeshShape>(collision_mesh.get(), true, true);

        btScalar mass = 0.0f;
        btVector3 local_inertia(0, 0, 0);

        btTransform transform;
        transform.setIdentity();
        auto& pos = model_transform.position;
        transform.setOrigin({pos.x, pos.y, pos.z});

        // Create the rigid body for the ground
        mesh_object.motion_state = std::make_unique<btDefaultMotionState>(transform);

        btRigidBody::btRigidBodyConstructionInfo rb_info(
            mass, mesh_object.motion_state.get(), mesh_object.collision_shape.get(), local_inertia);
        rb_info.m_friction = 1.25f;
        mesh_object.body = std::make_unique<btRigidBody>(rb_info);

        dynamics_world.addRigidBody(mesh_object.body.get());
    }

    // --------------------------------------------------------
    // ==== Bullet3D Experiments: Creates additional boxes ====
    // --------------------------------------------------------
    auto add_dynamic_shape = [&](const glm::vec3& position, const glm::vec3& force, float mass)
    {
        PhysicsObject& box = physics_objects.emplace_back();
        box.collision_shape = std::make_unique<btBoxShape>(btVector3{0.5f, 0.5f, 0.5f});

        btVector3 local_inertia(0, 0, 0);
        box.collision_shape->calculateLocalInertia(mass, local_inertia);

        btTransform bt_transform;
        bt_transform.setIdentity();
        bt_transform.setOrigin({position.x, position.y, position.z});

        box.motion_state = std::make_unique<btDefaultMotionState>(bt_transform);
        btRigidBody::btRigidBodyConstructionInfo box_rb_info(
            mass, box.motion_state.get(), box.collision_shape.get(), local_inertia);

        box_rb_info.m_friction = 0.9f;
        box.body = std::make_unique<btRigidBody>(box_rb_info);

        box.body->applyCentralForce({force.x, force.y, force.z});
        dynamics_world.addRigidBody(box.body.get());
    };

    DebugRenderer debug_renderer(camera);
    dynamics_world.setDebugDrawer(&debug_renderer);

    for (float y = 0.5; y < 10; y++)
    {
        for (float x = 2; x < 3; x++)
        {
            // add_dynamic_shape({x, y, 1}, {0, 0, 0});
        }
    }

    /*

            glCreateBuffers(1, &ubo);
            glNamedBufferStorage(ubo, sizeof(glm::mat4), nullptr, GL_DYNAMIC_STORAGE_BIT);

            GLuint matrix_index = glGetUniformBlockIndex(scene_shader.program_,
       "matrix_data1"); glUniformBlockBinding(scene_shader.program_, matrix_index, 0);

            glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
            glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo, 0, sizeof(glm::mat4));


            GLuint ubo2;
            glCreateBuffers(1, &ubo2);
            glNamedBufferStorage(ubo2, sizeof(glm::mat4), nullptr, GL_DYNAMIC_STORAGE_BIT);

            GLuint matrix_index2 = glGetUniformBlockIndex(scene_shader.program_,
       "matrix_data2"); glUniformBlockBinding(scene_shader.program_, matrix_index2, 1);

            glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo2);
            glBindBufferRange(GL_UNIFORM_BUFFER, 1, ubo2, 0, sizeof(glm::mat4));


            ...

            glNamedBufferSubData(ubo, 0, sizeof(view_matrix), &projection_matrix);
            glNamedBufferSubData(ubo2, 0, sizeof(view_matrix), &view_matrix);

        */

    // --------------
    // ==== SSBO ====
    // --------------
    BufferObject ubo;
    ubo.create_store(sizeof(glm::mat4) * 2);
    ubo.bind_buffer_base(BindBufferTarget::UniformBuffer, 0);
    ubo.bind_buffer_range(BindBufferTarget::UniformBuffer, 0, sizeof(glm::mat4) * 2);

    scene_shader.bind_uniform_block_index("matrix_data", 0);

    auto SIZE = sizeof(DirectionalLight) + sizeof(SpotLight);
    BufferObject light_ubo;
    light_ubo.create_store(SIZE);
    light_ubo.bind_buffer_base(BindBufferTarget::UniformBuffer, 1);
    light_ubo.bind_buffer_range(BindBufferTarget::UniformBuffer, 1, SIZE);

    BufferObject pointlights_ubo;
    pointlights_ubo.create_store(sizeof(PointLight) * 5);
    pointlights_ubo.bind_buffer_base(BindBufferTarget::UniformBuffer, 2);
    pointlights_ubo.bind_buffer_range(BindBufferTarget::UniformBuffer, 2, sizeof(PointLight) * 5);

    scene_shader.bind_uniform_block_index("Light", 1);
    scene_shader.bind_uniform_block_index("PointLights", 2);

    std::cout << "Size of DirectionalLight: " << sizeof(DirectionalLight) << '\n'
              << "Size of SpotLight:        " << sizeof(SpotLight) << '\n'
              << "Size of Point Light:      " << sizeof(PointLight) << '\n';

    // -------------------
    // ==== Main Loop ====
    // -------------------

    TimeStep<60> time_step;
    sf::Clock game_time;
    sf::Clock delta_clock;
    while (window.isOpen())
    {
        auto game_time_now = game_time.getElapsedTime();
        GUI::begin_frame();
        sf::Event e;
        while (window.pollEvent(e))
        {
            GUI::event(window, e);
            if (e.type == sf::Event::Closed)
            {
                window.close();
            }
            else if (e.type == sf::Event::KeyReleased)
            {
                if (e.key.code == sf::Keyboard::Escape)
                {
                    window.close();
                }
                else if (e.key.code == sf::Keyboard::L)
                {
                    mouse_locked = !mouse_locked;
                }
                else if (e.key.code == sf::Keyboard::B)
                {
                    float height = 15;
                    float width = 5;
                    float base = 50;
                    float start = 50;
                    for (float y = start; y < start + height; y++)
                    {
                        for (float x = base; x < base + width; x++)
                        {
                            add_dynamic_shape({x, y, base}, {0, 0, 0}, 1.0f);
                        }
                    }
                    for (float y = start; y < start + height; y++)
                    {
                        for (float x = base; x < base + width; x++)
                        {
                            add_dynamic_shape({x, y, base + width}, {0, 0, 0}, 1.0f);
                        }
                    }
                    for (float y = start; y < start + height; y++)
                    {
                        for (float z = base; z < base + width + 1; z++)
                        {
                            add_dynamic_shape({base - 1, y, z}, {0, 0, 0}, 1.0f);
                        }
                    }
                    for (float y = start; y < start + height; y++)
                    {
                        for (float z = base; z < base + width + 1; z++)
                        {
                            add_dynamic_shape({base + width, y, z}, {0, 0, 0}, 1.0f);
                        }
                    }
                }
                else if (e.key.code == sf::Keyboard::Space)
                {
                    auto& f = camera.get_forwards();
                    float x = f.x * settings.throw_force;
                    float y = f.y * settings.throw_force;
                    float z = f.z * settings.throw_force;

                    add_dynamic_shape(camera.transform.position + f * 3.0f, {x, y, z},
                                      settings.throw_mass);
                }
            }
        }
        if (!window.isOpen())
        {
            break;
        }
        // ImGui::SFML::Update()

        // ---------------
        // ==== Input ====
        // ---------------
        auto SPEED = 5.0f;
        auto translate = get_keyboard_input(camera.transform, true) * SPEED;

        if (!mouse_locked)
        {
            window.setMouseCursorVisible(false);
            get_mouse_move_input(camera.transform, window);
        }
        else
        {
            window.setMouseCursorVisible(true);
        }

        // ------------------------
        // ==== Sound handling ====
        // ------------------------
        // Walking sound effects
        if ((std::abs(translate.x + translate.y + translate.z) > 0) &&
            camera.transform.position.y > 0.5 && camera.transform.position.y < 1.5)
        {
            if (walk_sounds[sound_idx].getStatus() != sf::Sound::Status::Playing)
            {
                sound_idx++;
                if (sound_idx >= walk_sounds.size())
                {
                    sound_idx = 0;
                }

                walk_sounds[sound_idx].play();
            }
        }

        // ----------------------------------
        // ==== Update w/ Fixed timestep ====
        // ----------------------------------
        time_step.update(
            [&](auto dt)
            {
                camera.transform.position += translate * dt.asSeconds();

                light_transform.position.x +=
                    glm::sin(game_time_now.asSeconds() * 0.55f) * dt.asSeconds() * 3.0f;
                light_transform.position.z +=
                    glm::cos(game_time_now.asSeconds() * 0.55f) * dt.asSeconds() * 3.0f;
                light_transform.position.y =
                    height_map.get_height(static_cast<int>(light_transform.position.x),
                                          static_cast<int>(light_transform.position.z)) +
                    1.0f;
                //   settings.spot_light.cutoff -= 0.01;
            });

        // -------------------------
        // ==== Bullet3D Update ====
        // -------------------------
        dynamics_world.stepSimulation(1.0f / 60.0f, 10);

        // Remove dead objects
        for (auto itr = physics_objects.begin(); itr != physics_objects.end();)
        {
            auto& rb = itr->body;
            auto y = rb->getWorldTransform().getOrigin().getY();
            if (y < -5)
            {
                dynamics_world.removeRigidBody(itr->body.get());
                itr = physics_objects.erase(itr);
            }
            else
            {
                itr++;
            }
        }

        // Iterate through collisions?
        /*
        for (int i = 0; i < dynamics_world.getDispatcher()->getNumManifolds(); i++)
        {
            auto manifold = dynamics_world.getDispatcher()->getManifoldByIndexInternal(i);
            auto obj_a = manifold->getBody0();
            auto obj_b = manifold->getBody1();
            if (obj_a && obj_b)
            {
                if (manifold->getNumContacts() > 0)
                {
                    // collisoon...
                }
            }
        }
        */
        // -------------------------------
        // ==== Transform Calculations ====
        // -------------------------------
        // View/ Camera matrix
        camera.update();
        // Model matrices...
        auto terrain_mat = create_model_matrix(terrain_transform);
        auto light_mat = create_model_matrix(light_transform);

        // ------------------------------
        // ==== Set up shader states ====
        // ------------------------------
        //
        // GBuffer can be used here for deferred lighting
        // scene_shader.set_uniform("projection_matrix", camera.get_projection());
        // scene_shader.set_uniform("view_matrix", camera.get_view_matrix());
        ubo.buffer_sub_data(0, camera.get_projection());
        ubo.buffer_sub_data(sizeof(camera.get_view_matrix()), camera.get_view_matrix());

        scene_shader.set_uniform("material.diffuse0", 0);
        scene_shader.set_uniform("material.specular0", 1);

        // Scene
        scene_shader.set_uniform("eye_position", camera.transform.position);

        // For deferred shading:
        // scene_shader.set_uniform("postion_tex", 0);
        // scene_shader.set_uniform("normal_tex", 1);
        // scene_shader.set_uniform("albedo_spec_tex", 2);

        // clang-format off
        auto upload_base_light =
            [](Shader& shader, const LightBase& light, const std::string& uniform)
        {
            shader.set_uniform(uniform + ".base.colour",               light.colour);
            shader.set_uniform(uniform + ".base.ambient_intensity",    light.ambient_intensity);
            shader.set_uniform(uniform + ".base.diffuse_intensity",    light.diffuse_intensity);
            shader.set_uniform(uniform + ".base.specular_intensity",   light.specular_intensity);
        };
        auto upload_attenuation =
            [](Shader& shader, const Attenuation& attenuation, const std::string& uniform)
        {
            shader.set_uniform(uniform + ".att.constant",   attenuation.constant);
            shader.set_uniform(uniform + ".att.linear",     attenuation.linear);
            shader.set_uniform(uniform + ".att.exponant",   attenuation.exponant);
        };

        // Set the directional light shader uniforms
        DirectionalLight l = settings.lights.dir_light;
        light_ubo.buffer_sub_data(0, l);
        //scene_shader.set_uniform("dir_light.direction", settings.lights.dir_light.direction);
        //upload_base_light(scene_shader,                 settings.lights.dir_light, "dir_light");

        // Set the point light shader uniforms
        
        /*
        scene_shader.set_uniform("point_lights[0].position", glm::vec4(light_transform.position, 1.0f));
        upload_base_light(scene_shader,                     settings.lights.point_light, "point_lights[0]");
        upload_attenuation(scene_shader,                    settings.lights.point_light.att, "point_lights[0]");
        for (int i = 0; i < 5; i++)
        {
            auto pos = point_lights[i];
            pos.position += 1.0f;
            auto location = "point_lights[" + std::to_string(i + 1) + "]";
            scene_shader.set_uniform(location + ".position", pos.position);
        
        
            upload_base_light(scene_shader,                     settings.lights.point_light, location);
            upload_attenuation(scene_shader,                    settings.lights.point_light.att, location);
        }
        */
        scene_shader.set_uniform("light_count", 5);
        for (auto& light : point_lights) {
            auto p = light.position;
            light = settings.lights.point_light;
            light.position = p;
        }
        point_lights[4].position = glm::vec4(light_transform.position, 1.0f);
        pointlights_ubo.buffer_sub_data(0, point_lights);

        SpotLight s = settings.lights.spot_light;
        s.position = glm::vec4(camera.transform.position, 0.0f);
        s.direction = glm::vec4(camera.get_forwards(), 0.0f);
        s.cutoff = glm::cos(glm::radians(settings.lights.spot_light.cutoff));
        light_ubo.buffer_sub_data(sizeof(DirectionalLight), s);

        // Set the spot light shader uniforms
        //scene_shader.set_uniform("spot_light.cutoff",       glm::cos(glm::radians(settings.lights.spot_light.cutoff)));
       // scene_shader.set_uniform("spot_light.position",     glm::vec4(camera.transform.position, 0.0f));
        //scene_shader.set_uniform("spot_light.direction",    glm::vec4(camera.get_forwards(), 1.0f));
        //upload_base_light(scene_shader,                     settings.lights.spot_light, "spot_light");
       // upload_attenuation(scene_shader,                    settings.lights.spot_light.att, "spot_light");
        // clang-format on

        scene_shader.set_uniform("is_light", false);

        // -------------------------------------
        // ==== Render to GBuffer (fbo now) ====
        // -------------------------------------
        // gbuffer.bind();
        // gbuffer_shader.bind();

        if (settings.wireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, settings.wireframe ? GL_LINE : GL_FILL);
        }

        fbo.bind();
        scene_shader.bind();
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        // ==== Render Terrain ====
        if (settings.grass)
        {
            grass_material.bind();
        }
        else
        {
            create_material.bind();
        }

        scene_shader.set_uniform("model_matrix", terrain_mat);
        terrain_mesh.bind();
        terrain_mesh.draw();

        // Render the boxes, using the built in getOpenGLMatrix from bullet
        create_material.bind();
        box_vertex_mesh.bind();
        for (auto& box_transform : physics_objects)
        {
            glm::mat4 m{1.0f};
            box_transform.body->getWorldTransform().getOpenGLMatrix(glm::value_ptr(m));
            m = glm::translate(m, {-0.5, -0.5, -0.5});

            scene_shader.set_uniform("model_matrix", m);
            box_vertex_mesh.draw();
        }

        // ==== Render Billboards ====
        person_material.bind();
        billboard_vertex_array.bind();
        for (auto& transform : people_transforms)
        {
            // Draw billboard
            auto pi = static_cast<float>(std::numbers::pi);
            auto xd = transform.position.x - camera.transform.position.x;
            auto yd = transform.position.z - camera.transform.position.z;

            float r = std::atan2(xd, yd) + pi;

            glm::mat4 billboard_mat{1.0f};
            billboard_mat = glm::translate(billboard_mat, transform.position);
            billboard_mat = glm::rotate(billboard_mat, r, {0, 1, 0});

            scene_shader.set_uniform("model_matrix", billboard_mat);

            billboard_vertex_array.draw();
        }

        scene_shader.set_uniform("model_matrix", create_model_matrix(model_transform));
        model.draw(scene_shader);

        // ==== Render Floating Light ====
        scene_shader.set_uniform("is_light", true);
        scene_shader.set_uniform("model_matrix", light_mat);
        light_vertex_mesh.bind();
        light_vertex_mesh.draw();
        for (auto& light : point_lights)
        {
            glm::mat4 m{1.0f};
            m = glm::translate(m, {light.position.x, light.position.y, light.position.z});
            scene_shader.set_uniform("model_matrix", m);
            light_vertex_mesh.draw();
        }

        // Render debug stuff
        dynamics_world.debugDrawWorld();
        debug_renderer.render();

        // -----------------------
        // ==== Render to FBO ====
        // -----------------------
        /*
        fbo.bind();
        scene_shader.bind();
        gbuffer.bind_position_buffer_texture(0);
        gbuffer.bind_normal_buffer_texture(1);
        gbuffer.bind_albedo_specular_buffer_texture(2);

        */

        // --------------------------
        // ==== Render to window ====
        // --------------------------
        // Reset polygon mode to GL FILL for framebuffer
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // B i n d
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, window.getSize().x, window.getSize().y);

        // Bind the FBOs texture which will texture the screen quad
        fbo.bind_colour_attachment(0, 0);
        fbo_shader.bind();

        // Render
        glBindVertexArray(fbo_vbo);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // --------------------------
        // ==== End Frame ====
        // --------------------------
        // ImGui::ShowDemoWindow();
        GUI::debug_window(camera.transform.position, camera.transform.rotation, settings);
        GUI::debug_renderer_window(debug_renderer, settings);

        GUI::render();
        window.display();
    }

    // --------------------------
    // ==== Graceful Cleanup ====
    // --------------------------
    GUI::shutdown();
    glDeleteVertexArrays(1, &fbo_vbo);

    for (int i = dynamics_world.getNumCollisionObjects() - 1; i >= 0; i--)
    {
        btCollisionObject* obj = dynamics_world.getCollisionObjectArray()[i];
        dynamics_world.removeCollisionObject(obj);
    }
}