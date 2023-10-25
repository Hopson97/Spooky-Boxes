#include "Profiler.h"

#include <vector>

#include <imgui.h>

namespace
{
    template<int S>
    auto calculate_average(const CircluarQueue<sf::Time, S>& times)
    {   
        auto sum = sf::Time::Zero;
        for (auto time : times.data)
        {
            sum += time;
        }
        return sf::seconds(sum.asSeconds() / static_cast<float>(times.data.size()));
    }
}

ProfilerSection& Profiler::begin_section(const std::string& section)
{
    auto itr = profile_sections_.find(section);
    if (itr == profile_sections_.end())
    {
        itr = profile_sections_.emplace(section, ProfilerSection{}).first;
    }

    itr->second.clock.restart();
    return itr->second;
}

void Profiler::end_frame()
{
    frame_times_.push_back(frame_time_clock_.restart());
    frames_++;

    if (updater_timer_.getElapsedTime() > sf::seconds(0.25f))
    {
        updater_timer_.restart();

        for (auto& [name, section] : profile_sections_)
        {
            section.averge = calculate_average(section.times);
        }
        averge_ = calculate_average(frame_times_);
    }
}



void ProfilerSection::end_section()
{
    times.push_back(clock.getElapsedTime());
}


void Profiler::gui()
{
    if (ImGui::Begin("Profiler"))
    {
        ImGui::Text("Frame Time: %fms", averge_.asSeconds() * 1000);
        ImGui::Text("Frames: ", frames_);


        for (auto& [name, section] : profile_sections_)
        {
            ImGui::Separator();
            ImGui::Text("%s: %fms", name.c_str(), section.averge.asSeconds() * 1000);

            std::vector<float> times;

            for (auto& time : section.times.data)
            {
                times.push_back(time.asSeconds() * 1000);
            }
            ImGui::PlotLines("##Times", times.data(), times.size(), 0, nullptr, 0, 5, {200, 50});
        }
    }
    ImGui::End();
}