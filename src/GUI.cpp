#include "GUI.h"

#include <imgui_sfml/imgui-SFML.h>
#include <imgui_sfml/imgui_impl_opengl3.h>

#include "Graphics/DebugRenderer.h"
#include "Utils/Util.h"

namespace
{
    void base_light_widgets(LightBase& light)
    {
        ImGui::SliderFloat3("Colour", &light.colour[0], 0.0, 1.0);
        ImGui::SliderFloat("Ambient Intensity", &light.ambient_intensity, 0.0, 1.0);
        ImGui::SliderFloat("Diffuse Intensity", &light.diffuse_intensity, 0.0, 1.0);
        ImGui::SliderFloat("Specular Intensity", &light.specular_intensity, 0.0, 1.0);
    }

    void attenuation_widgets(Attenuation& attenuation)
    {
        ImGui::SliderFloat("Attenuation Constant", &attenuation.constant, 0.0, 1.0f);
        ImGui::SliderFloat("Attenuation Linear", &attenuation.linear, 0.14f, 0.0014f, "%.6f");
        ImGui::SliderFloat("Attenuation Quadratic", &attenuation.exponant, 0.000007f, 0.03f,
                           "%.6f");
    }
} // namespace

namespace GUI
{
    bool init(sf::Window* window)
    {
        if (!ImGui::SFML::Init(*window, cast_vector<float>(window->getSize())))
        {
            std::cerr << "Failed to init SFML Imgui.\n";
            return false;
        }

        if (!ImGui_ImplOpenGL3_Init())
        {
            std::cerr << "Failed to init SFML Imgui.\n";
            return false;
        }
        return true;
    }

    void begin_frame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
    }

    void shutdown()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui::SFML::Shutdown();
    }

    void end_frame()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void event(const sf::Window& window, sf::Event& e)
    {
        ImGui::SFML::ProcessEvent(window, e);
    }

    void debug_window(const glm::vec3& camera_position, const glm::vec3& camera_rotation,
                      Settings& settings)
    {
        // clang-format off
        if (ImGui::Begin("Debug Window"))
        {
            text_vec3("Position", camera_position);
            text_vec3("Rotation", camera_rotation);

            ImGui::SliderFloat("Material Shine", &settings.material_shine, 1.0f, 64.0f);

            ImGui::Separator();
            ImGui::Checkbox("Grass ground?", &settings.grass);

            ImGui::Separator();

            ImGui::PushID("DirLight");
            ImGui::Text("Directional light");
            if (ImGui::SliderFloat3("Direction", &settings.lights.dir_light.direction[0], -1.0, 1.0))
            {
                settings.lights.dir_light.direction = glm::normalize(settings.lights.dir_light.direction);
            }
            base_light_widgets(settings.lights.dir_light);
            ImGui::PopID();

            ImGui::Separator();

            ImGui::PushID("PointLight");
            ImGui::Text("Point light");
            base_light_widgets(settings.lights.point_light);
            attenuation_widgets(settings.lights.point_light.att);
            ImGui::PopID();

            ImGui::Separator();

            ImGui::PushID("SpotLight");
            ImGui::Text("Spot light");
            ImGui::SliderFloat("Cutoff", &settings.lights.spot_light.cutoff, 0.0, 90.0f);
            base_light_widgets(settings.lights.spot_light);
            attenuation_widgets(settings.lights.spot_light.att);
            ImGui::PopID();

            ImGui::SliderFloat("Throw Force", &settings.throw_force, 0.0, 10000.0f);
            ImGui::SliderFloat("Throw Mass", &settings.throw_mass, 0.0, 90.0f);

        }
        // clang-format on

        ImGui::End();
    }

    void text_vec3(const std::string& text, const glm::vec3& vect)
    {
        auto output = text + ": (%.2f, %.2f, %.2f)";
        ImGui::Text(output.c_str(), vect.x, vect.y, vect.z);
    }

} // namespace GUI