// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "glm/common.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <limits>
#include <random>
#if defined(_WIN32)
#include <windows.h>
#else
#include <pstl/glue_algorithm_defs.h>
#endif // _WIN32

#include "glad/glad.h"

#include "globals.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <cstddef>

#include <GLFW/glfw3.h>

#include <chrono>
#include <stdio.h>
#include <vector>
#include <queue>
#include <fstream>

#include <glm/gtc/type_ptr.hpp>

#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS
// compilers. To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of
// Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

#include "camera.h"
#include "gcode.h"
#include "shaders.h"

namespace glfwContext {
Camera camera;
int fps_target_value;

GLFWwindow *window{nullptr};
int         forth_back = 0;
int         left_right = 0;
int         up_down    = 0;
int         top_view   = 0;
bool middle_mouse_down = false;
double      last_xpos{0.0};
double      last_ypos{0.0};

static void glfw_error_callback(int error, const char *description) { fprintf(stderr, "GLFW Error %d: %s\n", error, description); }

static void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

    switch (action) {
    case GLFW_PRESS: {
        switch (key) {
        case GLFW_KEY_F2: {
            config::geometryMode = 1 - config::geometryMode;
            break;
        }
        case GLFW_KEY_F3: {
            config::vsync = 1 - config::vsync;
            std::cout << "vsync: " << config::vsync << std::endl;
            glfwSwapInterval(config::vsync);
            break;
        }
        case GLFW_KEY_W: {
            forth_back = 1;
            break;
        }
        case GLFW_KEY_S: {
            forth_back = -1;
            break;
        }
        case GLFW_KEY_A: {
            left_right = -1;
            break;
        }
        case GLFW_KEY_D: {
            left_right = 1;
            break;
        }
        case GLFW_KEY_Q: {
            up_down = 1;
            break;
        }
        case GLFW_KEY_Z: {
            up_down = -1;
            break;
        }
        case GLFW_KEY_T: {
            top_view = ('t');
            break;
        }
        case GLFW_KEY_KP_ADD: {
            camera.stepSize *= 2.0f;
            break;
        }
        case GLFW_KEY_KP_SUBTRACT: {
            camera.stepSize *= 0.5f;
            break;
        }
        case GLFW_KEY_LEFT: {
            sequential_range.decrease_current_max(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) ? 100 : 1);
            config::enabled_paths_update_required = true;
            break;
        }
        case GLFW_KEY_RIGHT: {
            sequential_range.increase_current_max(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) ? 100 : 1);
            config::enabled_paths_update_required = true;
            break;
        }
        case GLFW_KEY_DOWN: {
            sequential_range.decrease_current_min(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) ? 100 : 1);
            config::enabled_paths_update_required = true;
            break;
        }
        case GLFW_KEY_UP: {
            sequential_range.increase_current_min(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) ? 100 : 1);
            config::enabled_paths_update_required = true;
            break;
        }
        }
        break;
    }
    case GLFW_RELEASE: {
        switch (key) {
        case GLFW_KEY_W:
        case GLFW_KEY_S: {
            forth_back = 0;
            break;
        }
        case GLFW_KEY_A:
        case GLFW_KEY_D: {
            left_right = 0;
            break;
        }
        case GLFW_KEY_Q:
        case GLFW_KEY_Z: {
            up_down = 0;
            break;
        }
        case GLFW_KEY_T: {
            top_view = 't';
            break;
        }
        }
        break;
    }
    case GLFW_REPEAT: {
        switch (key) {
        case GLFW_KEY_LEFT: {
            sequential_range.decrease_current_max(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) ? 100 : 1);
            config::enabled_paths_update_required = true;
            break;
        }
        case GLFW_KEY_RIGHT: {
            sequential_range.increase_current_max(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) ? 100 : 1);
            config::enabled_paths_update_required = true;
            break;
        }
        case GLFW_KEY_DOWN: {
            sequential_range.decrease_current_min(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) ? 100 : 1);
            config::enabled_paths_update_required = true;
            break;
        }
        case GLFW_KEY_UP: {
            sequential_range.increase_current_min(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) ? 100 : 1);
            config::enabled_paths_update_required = true;
            break;
        }
        }
        break;
    }
    }
}

static void glfw_cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

    if (!ImGui::GetIO().WantCaptureMouse) {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) || glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
            glm::vec2 offset;
            offset[0] = (float)(-xpos + last_xpos);
            offset[1] = (float)(ypos - last_ypos);
            offset *= camera.rotationSpeed;
            camera.rotateCamera(offset);
        } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) || glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT)) {
            glm::vec2 offset;
            offset[0] = (float)(-xpos + last_xpos);
            offset[1] = (float) (ypos - last_ypos);
            offset *= camera.stepSize;
            camera.moveCamera({0.0, offset.x, offset.y});
        }
    }

    last_xpos = xpos;
    last_ypos = ypos;
}

static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
}

//The callback function receives two-dimensional scroll offsets.
void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.zoomCamera(-yoffset);
}

static void glfw_window_iconify_callback(GLFWwindow* window, int value)
{
    config::window_minimized = value == GLFW_TRUE;
}

} // namespace glfwContext

// Reader function to load vector of PathPoints from a file
static std::vector<gcode::PathPoint> readPathPoints(const std::string& filename)
{
    std::vector<gcode::PathPoint> pathPoints;

    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return pathPoints;
    }

    // Read the size of the vector
    size_t size;
    file.read(reinterpret_cast<char *>(&size), sizeof(size));
    pathPoints.resize(size);

//    std::map<globals::EMoveType, size_t> types_counters;

    // Read each PathPoint object from the file
    static const float travel_radius = 0.05f;
    static const float wipes_radius = 0.15f;
    for (auto &point : pathPoints) {
        file.read(reinterpret_cast<char *>(&point), sizeof(point));
        if (point.is_extrude_move())
            point.position.z -= 0.5f * point.height;
        else if (point.is_travel_move()) {
            point.width = travel_radius;
            point.height = travel_radius;
        }
        else if (point.is_wipe_move()) {
            point.width = wipes_radius;
            point.height = wipes_radius;
        }

//        const globals::EMoveType type = globals::extract_type_from_flags(point.flags);
//        auto it = types_counters.find(type);
//        if (it == types_counters.end())
//            it = types_counters.insert({ type, 0 }).first;
//        it->second++;
    }

    file.close();

    return pathPoints;
}

static void show_fps()
{
    ImGui::SetNextWindowPos({0.0f, 0.0f}, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("##fps", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
    ImGui::Text("FPS %d", (unsigned int) ImGui::GetCurrentContext()->IO.Framerate);
    ImGui::End();
    ImGui::PopStyleVar();
}

void show_config_window()
{
    int width, height;
    glfwGetWindowSize(glfwContext::window, &width, &height);

    ImGui::SetNextWindowPos({ (float)width, 0.0f }, ImGuiCond_Always, { 1.0, 0.0 });
    ImGui::SetNextWindowBgAlpha(0.25f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("##config", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
    if (ImGui::Checkbox("force_full_model_render", &config::force_full_model_render))
         config::enabled_paths_update_required = true;
    if (ImGui::Checkbox("vsync", &config::vsync)) {}
    if (ImGui::Checkbox("use_travel_moves_data", &config::use_travel_moves_data)) {
        config::ranges_update_required = true;
        config::color_update_required = true;
    }
    ImGui::Separator();
    if (ImGui::Button("Center view", { -1.0f, 0.0f }))
        config::camera_center_required = true;

    ImGui::Text("Keep FPS above: ");
    ImGui::SameLine();
    ImGui::SliderInt("##slider_low", &glfwContext::fps_target_value, 0, 60, "%d", ImGuiSliderFlags_NoInput);

    ImGui::End();
    ImGui::PopStyleVar();
}

static void show_opengl()
{
    int width, height;
    glfwGetWindowSize(glfwContext::window, &width, &height);

    ImGui::SetNextWindowPos({ 0.0f, (float)height }, ImGuiCond_Always, { 0.0f, 1.0f });
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("##opengl", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
    const char* value = (const char*)::glGetString(GL_VERSION);
    ImGui::Text("OpenGL version: %s", value);
    int profile = 0;
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);
    ImGui::SameLine();
    ImGui::Text((profile == GL_CONTEXT_CORE_PROFILE_BIT) ? "Core" : "Compatibility");

    ImGui::End();
    ImGui::PopStyleVar();
}

static void show_visualization_type()
{
    int width, height;
    glfwGetWindowSize(glfwContext::window, &width, &height);

    ImGui::SetNextWindowPos({ (float)width, (float)height }, ImGuiCond_Always, { 1.0f, 1.0f });
    ImGui::SetNextWindowBgAlpha(0.25f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("##visualization_type", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

    const char* options[] = {
        "Feature type",
        "Height (mm)",
        "Width (mm)",
        "Speed (mm/s)",
        "Fan speed (%)",
        "Temperature (�C)",
        "Volumetric flow rate (mm�/s)",
        "Layer time (linear)",
        "Layer time (logarithmic)",
        "Tool",
        "Color Print"
    };

    const int old_visualization_type = config::visualization_type;
    if (ImGui::Combo("##combo", &config::visualization_type, options, IM_ARRAYSIZE(options))) {
        if (old_visualization_type != config::visualization_type)
            config::color_update_required = true;
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void show_sequential_sliders()
{
    int width, height;
    glfwGetWindowSize(glfwContext::window, &width, &height);

    ImGui::SetNextWindowPos({ 0.5f * (float)width, (float)height }, ImGuiCond_Always, { 0.5f, 1.0f });
    ImGui::SetNextWindowBgAlpha(0.25f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("##sequential", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

    const int global_min = (int)sequential_range.get_global_min();
    const int global_max = (int)sequential_range.get_global_max();

    const std::string start = "1";
    const std::string end = std::to_string(global_max);
    const float labels_width = ImGui::CalcTextSize((start + end).c_str()).x;

    ImGui::Text("%d", 1);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(0.5f * width - labels_width);
    int low = (int)sequential_range.get_current_min();
    if (ImGui::SliderInt("##slider_low", &low, global_min, global_max, "%d", ImGuiSliderFlags_NoInput)) {
        sequential_range.set_current_min((size_t)low);
        config::enabled_paths_update_required = true;
    }
    ImGui::SameLine();
    ImGui::Text("%d", global_max);

    ImGui::Text("%d", 1);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(0.5f * width - labels_width);
    int high = (int)sequential_range.get_current_max();
    if (ImGui::SliderInt("##slider_high", &high, global_min, global_max, "%d", ImGuiSliderFlags_NoInput)) {
        sequential_range.set_current_max((size_t)high);
        config::enabled_paths_update_required = true;
    }
    ImGui::SameLine();
    ImGui::Text("%d", global_max);

    ImGui::End();
    ImGui::PopStyleVar();
}

void show_extrusion_roles()
{
    int width, height;
    glfwGetWindowSize(glfwContext::window, &width, &height);

    ImGui::SetNextWindowPos({ (float)width, 0.5f * (float)height }, ImGuiCond_Always, { 1.0f, 0.5f });
    ImGui::SetNextWindowBgAlpha(0.25f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("##extrusion_roles", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

    auto append_extrusion_role_checkbox = [](globals::GCodeExtrusionRole role) {
        bool& value = config::extrusion_roles_visibility[role];
        if (ImGui::Checkbox(globals::gcode_extrusion_role_to_string(role).c_str(), &value)) { config::enabled_paths_update_required = true; }
    };
    auto append_option_checkbox = [](globals::EMoveType type) {
        bool& value = config::options_visibility[type];
        if (ImGui::Checkbox(globals::gcode_move_type_to_string(type).c_str(), &value)) { config::enabled_paths_update_required = true; }
    };

    append_extrusion_role_checkbox(globals::GCodeExtrusionRole::None);
    append_extrusion_role_checkbox(globals::GCodeExtrusionRole::Perimeter);
    append_extrusion_role_checkbox(globals::GCodeExtrusionRole::ExternalPerimeter);
    append_extrusion_role_checkbox(globals::GCodeExtrusionRole::OverhangPerimeter); 
    append_extrusion_role_checkbox(globals::GCodeExtrusionRole::InternalInfill); 
    append_extrusion_role_checkbox(globals::GCodeExtrusionRole::SolidInfill); 
    append_extrusion_role_checkbox(globals::GCodeExtrusionRole::TopSolidInfill); 
    append_extrusion_role_checkbox(globals::GCodeExtrusionRole::Ironing); 
    append_extrusion_role_checkbox(globals::GCodeExtrusionRole::BridgeInfill); 
    append_extrusion_role_checkbox(globals::GCodeExtrusionRole::GapFill); 
    append_extrusion_role_checkbox(globals::GCodeExtrusionRole::Skirt); 
    append_extrusion_role_checkbox(globals::GCodeExtrusionRole::SupportMaterial); 
    append_extrusion_role_checkbox(globals::GCodeExtrusionRole::SupportMaterialInterface); 
    append_extrusion_role_checkbox(globals::GCodeExtrusionRole::WipeTower); 
    append_extrusion_role_checkbox(globals::GCodeExtrusionRole::Custom); 
    ImGui::Separator();
    if (ImGui::Checkbox("Travel", &config::travel_paths_visibility))
        config::enabled_paths_update_required = true;
    append_option_checkbox(globals::EMoveType::Retract);
    append_option_checkbox(globals::EMoveType::Unretract);
    append_option_checkbox(globals::EMoveType::Seam);
    append_option_checkbox(globals::EMoveType::Tool_change);
    append_option_checkbox(globals::EMoveType::Color_change);
    append_option_checkbox(globals::EMoveType::Pause_print);
    append_option_checkbox(globals::EMoveType::Custom_GCode);
    append_option_checkbox(globals::EMoveType::Wipe);

    ImGui::End();
    ImGui::PopStyleVar();
}

void show_statistics()
{
    int width, height;
    glfwGetWindowSize(glfwContext::window, &width, &height);

    ImGui::SetNextWindowPos({ 0.0f, 0.5f * (float)height }, ImGuiCond_Always, { 0.0f, 0.5f });
    ImGui::SetNextWindowBgAlpha(0.25f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("##statistics", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

    const ImVec4 label_color(1.0f, 1.0f, 0.0f, 1.0f);
    const ImVec4 value_color(1.0f, 1.0f, 1.0f, 1.0f);

    auto add_memsize_row = [&](const std::string& label, size_t size) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        const std::string key = label + " size";
        ImGui::TextColored(label_color, key.c_str());
        ImGui::TableSetColumnIndex(1);
        const auto [mem_size, units] = globals::format_memsize(size);
        if (units == "bytes")
            ImGui::TextColored(value_color, "%d %s", size, units.c_str());
        else
            ImGui::TextColored(value_color, "%.3f %s", mem_size, units.c_str());
    };

    if (ImGui::BeginTable("Statistics", 2)) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextColored(label_color, "Window size");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextColored(value_color, "%dx%d", width, height);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextColored(label_color, "Total moves");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextColored(value_color, "%d", globals::statistics.total_moves);
        add_memsize_row("Moves", globals::statistics.moves_size);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextColored(label_color, "Enabled lines");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextColored(value_color, "%d", globals::statistics.enabled_lines);
        add_memsize_row("Enabled lines buffer", globals::statistics.enabled_lines_size);
        add_memsize_row("Colors buffer", globals::statistics.colors_size);
        add_memsize_row("Positions buffer", globals::statistics.positions_size);
        add_memsize_row("HWA buffer", globals::statistics.height_width_angle_size);
        add_memsize_row("Vertex data buffer", globals::statistics.vertex_data_size);
        add_memsize_row("Total buffers", globals::statistics.buffers_size());
        ImGui::EndTable();
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

namespace rendering {


class SceneBox
{
    glm::vec3 m_min{ FLT_MAX, FLT_MAX, FLT_MAX };
    glm::vec3 m_max{ -FLT_MAX, -FLT_MAX, -FLT_MAX };

public:
    void update(const std::vector<gcode::PathPoint>& points) {
        for (const gcode::PathPoint& p : points) {
            m_min.x = std::min(m_min.x, p.position.x);
            m_min.y = std::min(m_min.y, p.position.y);
            m_min.z = std::min(m_min.z, p.position.z);
            m_max.x = std::max(m_max.x, p.position.x);
            m_max.y = std::max(m_max.y, p.position.y);
            m_max.z = std::max(m_max.z, p.position.z);
        }
    }

    void center_camera() {
        glfwContext::camera.target =  0.5f * (m_min + m_max);
        glfwContext::camera.position = glfwContext::camera.target + glm::length(m_max - m_min) * glm::normalize(glm::vec3(-1.0f, -1.0f, 1.0f));
    }

    glm::vec3 get_size() {
        return glm::abs(m_max - m_min);
    }
};

SceneBox scene_box;

static size_t hope_unique(uint32_t *out, size_t len) {
    if(len ==  0) return 0; // duh!
    size_t pos = 1;
    uint32_t oldv = out[0];
    for (size_t i = 1; i < len; ++i) {
        uint32_t newv = out[i];
        out[pos] = newv;
        pos += (newv != oldv);
        oldv = newv;
    }
    return pos;
}

Camera camera_snapshot = glfwContext::camera;

void switchConfiguration()
{
    if (config::geometryMode) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        checkGl();
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        checkGl();
    }
}

void render(gcode::BufferedPath &path)
{
    glfwContext::camera.moveCamera({glfwContext::forth_back, glfwContext::left_right, glfwContext::up_down});

    if (config::camera_center_required) {
        scene_box.center_camera();
        config::camera_center_required = false;
    }

    glm::ivec2 new_size;
    glfwGetFramebufferSize(glfwContext::window, &new_size.x, &new_size.y);
    checkGl();
    if (new_size != globals::screenResolution) {
        globals::screenResolution     = new_size;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkGl();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    checkGl();
    glViewport(0, 0, globals::screenResolution.x, globals::screenResolution.y);
    checkGl();

    glUseProgram(shaderProgram::gcode_program);
    glBindVertexArray(gcode::gcodeVAO);
    checkGl();

    const int positions_tex_id = ::glGetUniformLocation(shaderProgram::gcode_program, "positionsTex");
    assert(positions_tex_id >= 0);
    const int height_width_angle_tex_id = ::glGetUniformLocation(shaderProgram::gcode_program, "heightWidthAngleTex");
    assert(height_width_angle_tex_id >= 0);
     const int colors_tex_id = ::glGetUniformLocation(shaderProgram::gcode_program, "colorsTex");
    assert(colors_tex_id >= 0);
    const int segment_index_tex_id = ::glGetUniformLocation(shaderProgram::gcode_program, "segmentIndexTex");
    assert(segment_index_tex_id >= 0);

    glUniform1i(positions_tex_id, 0);
    glUniform1i(height_width_angle_tex_id, 1);
    glUniform1i(colors_tex_id, 2);
    glUniform1i(segment_index_tex_id, 3);
    checkGl();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_BUFFER, path.positions_texture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, path.positions_buffer);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_BUFFER, path.height_width_angle_texture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, path.height_width_angle_buffer);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_BUFFER, path.color_texture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, path.color_buffer);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_BUFFER, path.enabled_lines_texture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, path.enabled_lines_buffer);

    const int vp_id = ::glGetUniformLocation(shaderProgram::gcode_program, "view_projection");
    assert(vp_id >= 0);
    const int camera_forward_id = ::glGetUniformLocation(shaderProgram::gcode_program, "camera_forward");
    assert(camera_forward_id >= 0);

    auto view_projection = glfwContext::camera.get_view_projection();
    glUniformMatrix4fv(vp_id, 1, GL_FALSE, glm::value_ptr(view_projection));
    auto camera_forward = glfwContext::camera.forward;
    glUniform3fv(camera_forward_id, 1, glm::value_ptr(camera_forward));
    checkGl();

    if (path.enabled_lines_count > 0)
        glDrawArraysInstanced(GL_TRIANGLES, 0, (GLsizei)gcode::vertex_data.size(), path.enabled_lines_count);
    checkGl();

    glUseProgram(0);
    glBindVertexArray(0);
}

void setup()
{
    shaderProgram::createGCodeProgram();
    gcode::init();
    checkGl();
}
} // namespace rendering

// Main code
int main(int argc, char *argv[])
{
    // Check if a filename argument is provided
    if (argc < 2) {
        std::cout << "Please provide a filename as an argument." << std::endl;
        return 1;
    }

    // Read the filename from argv[1]
    const std::string filename = argv[1];

    auto points = readPathPoints(filename);
    rendering::scene_box.update(points);

    globals::statistics.total_moves = points.size();
    globals::statistics.moves_size = points.size() * sizeof(gcode::PathPoint);

    glfwSetErrorCallback(glfwContext::glfw_error_callback);
    if (!glfwInit())
        return 1;

    const GLFWvidmode* video_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    assert(video_mode != nullptr);

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char *glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char *glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac
#else
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);  // 3.2+ only    
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
    glfwWindowHint(GLFW_VISIBLE, false);

    // Create window with graphics context
    // 1284x883 is the size of the canvas in PrusaSlicer preview on 1920x1080 monitor
    // 1788x916 is the size of the canvas in GCodeViewer on 1920x1080 monitor
    const int window_w = 1284; // 1788;
    const int window_h = 883;  // 916;
    GLFWwindow* window = glfwCreateWindow(window_w, window_h, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (window == nullptr)
        return 1;

    glfwSetWindowPos(window, (video_mode->width - window_w) / 2, (video_mode->height - window_h) / 2);
    glfwShowWindow(window);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    glfwContext::window = window;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.IniFilename = nullptr;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    glfwSetKeyCallback(window, glfwContext::glfw_key_callback);
    glfwSetCursorPosCallback(window, glfwContext::glfw_cursor_position_callback);
    glfwSetMouseButtonCallback(window, glfwContext::glfw_mouse_button_callback);
    glfwSetWindowIconifyCallback(window, glfwContext::glfw_window_iconify_callback);
    glfwSetScrollCallback(window, glfwContext::glfw_scroll_callback);


    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont()
    // to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an
    // assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling
    // ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten
    // for details.
    // io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    // IM_ASSERT(font != nullptr);

    rendering::setup();

    // gcode::BufferedPath path = gcode::generateTestingPathPoints();
    gcode::BufferedPath path = gcode::bufferExtrusionPaths(points);

    sequential_range.set_global_max(path.total_points_count);
    sequential_range.set_current_max(path.total_points_count);

    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of
        // the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy
        // of the keyboard data. Generally you may always pass all inputs to dear imgui, and hide them from your application based on those
        // two flags.
        glfwPollEvents();

        if (glfwWindowShouldClose(window))
            break;

        if (config::window_minimized)
            continue;

        rendering::switchConfiguration();

        if (config::ranges_update_required) {
            gcode::set_ranges(points);
            config::ranges_update_required = false;
        }

        if (config::color_update_required) {
            gcode::updatePathColors(path, points);
            config::color_update_required = false;
        }

         if (config::enabled_paths_update_required) {
            gcode::updateEnabledLines(path, points);
            config::enabled_paths_update_required = false;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        show_fps();
        show_config_window();
        show_opengl();
        show_visualization_type();
        show_extrusion_roles();
        show_sequential_sliders();
        show_statistics();

        rendering::render(path);

        // Rendering
        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
