#include <array>
#include <numbers>

#include <SFML/Graphics/Image.hpp>
#include <SFML/Window/Event.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "GUI.h"
#include "Graphics/Lights.h"
#include "Graphics/Mesh.h"
#include "Graphics/OpenGL/Framebuffer.h"
#include "Graphics/OpenGL/GLDebugEnable.h"
#include "Graphics/OpenGL/GLResource.h"
#include "Graphics/OpenGL/Shader.h"
#include "Graphics/OpenGL/Texture.h"
#include "Graphics/OpenGL/VertexArray.h"
#include "Maths.h"
#include "Util.h"

#include <imgui.h>

#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

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
        sf::Mouse::setPosition({(int)window.getSize().x / 2, (int)window.getSize().y / 2},
                               window);
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

    // ----------------------------------------
    // ==== Create the Meshes + OpenGL vertex array ====
    // ----------------------------------------
    VertexArray billboard_vertex_array{generate_quad_mesh(1.0f, 2.0f)};
    VertexArray terrain_vertex_array{generate_terrain_mesh(128)};
    VertexArray light_vertex_array{generate_cube_mesh({0.2f, 0.2f, 0.2f})};
    VertexArray box_vertex_array{generate_cube_mesh({2.0f, 2.0f, 2.0f})};

    // ------------------------------------
    // ==== Create the OpenGL Textures ====
    // ------------------------------------
    Material person_material("assets/textures/person.png",
                             "assets/textures/person_specular.png");
    Material grass_material("assets/textures/grass_03.png",
                            "assets/textures/grass_specular.png");

    Material create_material("assets/textures/crate.png",
                             "assets/textures/crate_specular.png");

    // ---------------------------------------
    // ==== Create the OpenGL Framebuffer ====
    // ---------------------------------------
    Framebuffer fbo(window.getSize().x, window.getSize().y);
    fbo.attach_colour().attach_renderbuffer();
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
    if (!scene_shader.load_from_file("assets/shaders/SceneVertex.glsl",
                                     "assets/shaders/SceneFragment.glsl"))
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
    Transform camera_transform;
    Transform terrain_transform;
    Transform light_transform;
    std::vector<Transform> box_transforms;
    for (int i = 0; i < 25; i++)
    {
        float x = static_cast<float>(rand() % 120) + 3;
        float z = static_cast<float>(rand() % 120) + 3;
        float r = static_cast<float>(rand() % 360);

        box_transforms.push_back({{x, 0.0f, z}, {0.0f, r, 0}});
    }

    std::vector<Transform> people_transforms;
    for (int i = 0; i < 50; i++)
    {
        float x = static_cast<float>(rand() % 120) + 3;
        float z = static_cast<float>(rand() % 120) + 3;

        people_transforms.push_back({{x, 0.0f, z}, {0.0f, 0.0, 0}});
    }

    camera_transform.position = {80.0f, 1.0f, 35.0f};
    camera_transform.rotation = {0.0f, 201.0f, 0.0f};
    light_transform.position = {20.0f, 5.0f, 20.0f};

    glm::mat4 camera_projection =
        create_projection_matrix(window.getSize().x, window.getSize().y, 75.0f);
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
        [](sf::Music& background_sfx, const std::string path, int volume, int offset)
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
    create_looping_bg(ambient_night1, "assets/sounds/crickets.ogg", 50, 0);
    create_looping_bg(ambient_night2, "assets/sounds/crickets.ogg", 50, 5);
    create_looping_bg(spookysphere, "assets/sounds/Atmosphere_003(Loop).wav", 10, 0);

    // -------------------
    // ==== Main Loop ====
    // -------------------
    Settings settings;

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
        auto translate = get_keyboard_input(camera_transform, true) * SPEED;

        if (!mouse_locked)
        {
            window.setMouseCursorVisible(false);
            get_mouse_move_input(camera_transform, window);
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
            camera_transform.position.y > 0.5 && camera_transform.position.y < 1.5)
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
                camera_transform.position += translate * dt.asSeconds();

                light_transform.position.x +=
                    glm::sin(game_time_now.asSeconds() * 0.55f) * dt.asSeconds() * 3.0f;
                light_transform.position.z +=
                    glm::cos(game_time_now.asSeconds() * 0.55f) * dt.asSeconds() * 3.0f;

                //   settings.spot_light.cutoff -= 0.01;
            });

        // -------------------------------
        // ==== Transform Calculations ====
        // -------------------------------
        // View/ Camera matrix
        glm::mat4 view_matrix = create_view_matrix(camera_transform, {0.0f, 1.0f, 0.0f});
        auto x_rot = glm::radians(camera_transform.rotation.x);
        auto y_rot = glm::radians(camera_transform.rotation.y);
        glm::vec3 front = {
            glm::cos(y_rot) * glm::cos(x_rot),
            glm::sin(x_rot),
            glm::sin(y_rot) * glm::cos(x_rot),
        };
        // Model matrices...
        auto terrain_mat = create_model_matrix(terrain_transform);
        auto light_mat = create_model_matrix(light_transform);

        std::vector<glm::mat4> box_mats;
        for (auto& box_transform : box_transforms)
        {
            box_mats.push_back(create_model_matrix(box_transform));
        }

        // -----------------------
        // ==== Render to FBO ====
        // -----------------------
        // Set the framebuffer as the render target and clear
        fbo.bind();
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        // Set the shader states
        //......................
        scene_shader.bind();
        scene_shader.set_uniform("projection_matrix", camera_projection);
        scene_shader.set_uniform("view_matrix", view_matrix);

        scene_shader.set_uniform("eye_position", camera_transform.position);

        scene_shader.set_uniform("material.diffuse0", 0);
        scene_shader.set_uniform("material.specular0", 1);
        scene_shader.set_uniform("material.shininess", settings.material_shine);
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
        scene_shader.set_uniform("dir_light.direction", settings.dir_light.direction);
        upload_base_light(scene_shader,                 settings.dir_light, "dir_light");

        // Set the point light shader uniforms
        scene_shader.set_uniform("point_light.position", light_transform.position);
        upload_base_light(scene_shader,                     settings.point_light, "point_light");
        upload_attenuation(scene_shader,                    settings.point_light.att, "point_light");

        // Set the spot light shader uniforms
        scene_shader.set_uniform("spot_light.cutoff",       glm::cos(glm::radians(settings.spot_light.cutoff)));
        scene_shader.set_uniform("spot_light.position",     camera_transform.position);
        scene_shader.set_uniform("spot_light.direction",    front);
        upload_base_light(scene_shader,                     settings.spot_light, "spot_light");
        upload_attenuation(scene_shader,                    settings.spot_light.att, "spot_light");
        // clang-format on

        scene_shader.set_uniform("is_light", false);

        // Set the terrain trasform and render
        if (settings.grass)
        {
            grass_material.bind();
        }
        else
        {
            create_material.bind();
        }

        scene_shader.set_uniform("model_matrix", terrain_mat);
        terrain_vertex_array.bind();
        terrain_vertex_array.draw();

        // Set the box transforms and render
        create_material.bind();
        box_vertex_array.bind();
        for (auto& box_matrix : box_mats)
        {
            scene_shader.set_uniform("model_matrix", box_matrix);
            box_vertex_array.draw();
        }

        // Draw billboards
        person_material.bind();
        billboard_vertex_array.bind();
        for (auto& transform : people_transforms)
        {

            // Draw billboard
            auto pi = static_cast<float>(std::numbers::pi);
            auto xd = transform.position.x - camera_transform.position.x;
            auto yd = transform.position.z - camera_transform.position.z;

            float r = std::atan2(xd, yd) + pi;

            glm::mat4 billboard_mat{1.0f};
            billboard_mat = glm::translate(billboard_mat, transform.position);
            billboard_mat = glm::rotate(billboard_mat, r, {0, 1, 0});

            scene_shader.set_uniform("model_matrix", billboard_mat);

            billboard_vertex_array.draw();
        }

        // Set the light trasform and render
        scene_shader.set_uniform("is_light", true);
        scene_shader.set_uniform("model_matrix", light_mat);
        light_vertex_array.bind();
        light_vertex_array.draw();

        // --------------------------
        // ==== Render to window ====
        // --------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, window.getSize().x, window.getSize().y);

        // Bind the FBOs texture which will texture the screen quad
        fbo.bind_colour_attachment(0, 0);
        glBindVertexArray(fbo_vbo);
        fbo_shader.bind();

        // Render
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // --------------------------
        // ==== End Frame ====
        // --------------------------
        // ImGui::ShowDemoWindow();
        GUI::debug_window(camera_transform.position, camera_transform.rotation, settings);

        GUI::render();
        window.display();
    }

    // --------------------------
    // ==== Graceful Cleanup ====
    // --------------------------
    GUI::shutdown();
}