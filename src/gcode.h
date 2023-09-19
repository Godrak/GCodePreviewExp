#ifndef GCODE_H_
#define GCODE_H_

#include <glm/fwd.hpp>
#include "bitset.h"
#include "glad/glad.h"
#include "glm/geometric.hpp"
#include "globals.h"
#include "camera.h"

#include <cstddef>
#include <math.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <random>
#include <iostream>
#include <algorithm>
#include <array>
#include <limits>

namespace gcode {

GLuint gcodeVAO, vertexBuffer;

GLint vid_loc = 0;

#if ENABLE_ALTERNATE_SEGMENT_GEOMETRY
// see:
// doc/1.png
// doc/2.png
// doc/3.png
// for a visual description of vertices
const std::vector<uint8_t> vertex_data = {
    1, 6, 7, // stem
    1, 7, 2, // stem
    2, 7, 8, // stem 
    2, 8, 3, // stem 
    3, 8, 9, // stem 
    3, 9, 4, // stem 
    4, 9, 6, // stem
    4, 6, 1, // stem

    0, 1, 2, // cap 1
    0, 2, 3, // cap 1
    0, 3, 4, // cap 1
    0, 4, 1, // cap 1

    5, 8, 7, // cap 2
    5, 7, 6, // cap 2
    5, 6, 9, // cap 2
    5, 9, 8, // cap 2
};
#else
//     /1-------6\    
//    / |       | \  
//   2--0-------5--7
//    \ |       | /  
//      3-------4    
const std::vector<uint8_t> vertex_data = {
    0, 1, 2, // front spike
    0, 2, 3, // front spike
    0, 3, 4, // right/bottom body 
    0, 4, 5, // right/bottom body 
    0, 5, 6, // left/top body 
    0, 6, 1, // left/top body 
    5, 4, 7, // back spike
    5, 7, 6, // back spike
};
#endif // ENABLE_ALTERNATE_SEGMENT_GEOMETRY

struct PathPoint
{
    glm::vec3 position;
    unsigned int flags;
    float height;
    float width;
    float speed;
    float fanspeed;
    float temperature;
    float volumetricrate;
    unsigned int extruderid;
    unsigned int colorid;

    unsigned int encode_flags(unsigned int role, unsigned int type) { flags = role << 0 | type << 8; return flags; }
    globals::GCodeExtrusionRole role_from_flags() const { return globals::extract_role_from_flags(flags); }
    globals::EMoveType type_from_flags() const { return globals::extract_type_from_flags(flags); }

    bool is_extrude_move() const { return globals::extract_type_from_flags(flags) == globals::EMoveType::Extrude; }
    bool is_custom_gcode_move() const {
        return globals::extract_type_from_flags(flags) == globals::EMoveType::Extrude &&
               globals::extract_role_from_flags(flags) == globals::GCodeExtrusionRole::Custom;
    }
    bool is_travel_move() const { return globals::extract_type_from_flags(flags) == globals::EMoveType::Travel; }
    bool is_wipe_move() const { return globals::extract_type_from_flags(flags) == globals::EMoveType::Wipe; }
    bool is_option_move() const {
        switch (globals::extract_type_from_flags(flags))
        {
        case globals::EMoveType::Retract:
        case globals::EMoveType::Unretract:
        case globals::EMoveType::Seam:
        case globals::EMoveType::Tool_change:
        case globals::EMoveType::Color_change:
        case globals::EMoveType::Pause_print:
        case globals::EMoveType::Wipe:
        {
            return true;
        }
        default:
        {
            return false;
        }
        }
    }
};

const std::array<float, 3> Dummy_Color{ 0.0f, 0.0f, 0.0f };

const std::map<globals::EMoveType, std::array<float, 3>> Option_Colors{ {
    { globals::EMoveType::Retract,      { 0.803f, 0.135f, 0.839f } },
    { globals::EMoveType::Unretract,    { 0.287f, 0.679f, 0.810f } },
    { globals::EMoveType::Seam,         { 0.900f, 0.900f, 0.900f } },
    { globals::EMoveType::Tool_change,  { 0.758f, 0.744f, 0.389f } },
    { globals::EMoveType::Color_change, { 0.856f, 0.582f, 0.546f } },
    { globals::EMoveType::Pause_print,  { 0.322f, 0.942f, 0.512f } },
    { globals::EMoveType::Custom_GCode, { 0.886f, 0.825f, 0.262f } },
    { globals::EMoveType::Wipe,         { 1.000f, 1.000f, 0.000f } }
} };

const std::vector<std::array<float, 3>> Extrusion_Role_Colors{ {
    { 0.90f, 0.70f, 0.70f },   // None
    { 1.00f, 0.90f, 0.30f },   // Perimeter
    { 1.00f, 0.49f, 0.22f },   // ExternalPerimeter
    { 0.12f, 0.12f, 1.00f },   // OverhangPerimeter
    { 0.69f, 0.19f, 0.16f },   // InternalInfill
    { 0.59f, 0.33f, 0.80f },   // SolidInfill
    { 0.94f, 0.25f, 0.25f },   // TopSolidInfill
    { 1.00f, 0.55f, 0.41f },   // Ironing
    { 0.30f, 0.50f, 0.73f },   // BridgeInfill
    { 1.00f, 1.00f, 1.00f },   // GapFill
    { 0.00f, 0.53f, 0.43f },   // Skirt
    { 0.00f, 1.00f, 0.00f },   // SupportMaterial
    { 0.00f, 0.50f, 0.00f },   // SupportMaterialInterface
    { 0.70f, 0.89f, 0.67f },   // WipeTower
    { 0.37f, 0.82f, 0.58f },   // Custom
} };

const std::vector<std::array<float, 3>> Travel_Colors{ {
    { 0.219f, 0.282f, 0.609f }, // Move
    { 0.112f, 0.422f, 0.103f }, // Extrude
    { 0.505f, 0.064f, 0.028f }  // Retract
} };

const std::vector<std::array<float, 3>> Range_Colors{ {
    { 0.043f, 0.173f, 0.478f }, // bluish
    { 0.075f, 0.349f, 0.522f },
    { 0.110f, 0.533f, 0.569f },
    { 0.016f, 0.839f, 0.059f },
    { 0.667f, 0.949f, 0.000f },
    { 0.988f, 0.975f, 0.012f },
    { 0.961f, 0.808f, 0.039f },
    { 0.890f, 0.533f, 0.125f },
    { 0.820f, 0.408f, 0.188f },
    { 0.761f, 0.322f, 0.235f },
    { 0.581f, 0.149f, 0.087f }  // reddish
} };

const std::vector<std::array<float, 3>> Tools_Colors{ {
    { 1.000f, 0.502f, 0.000f },
    { 0.859f, 0.318f, 0.510f },
    { 0.243f, 0.753f, 1.000f },
    { 1.000f, 0.310f, 0.310f },
    { 0.984f, 0.922f, 0.490f }
} };

class Range
{
    float m_min{ FLT_MAX };
    float m_max{ -FLT_MAX };

public:
    void reset() { m_min = FLT_MAX; m_max = -FLT_MAX; }

    void update(float value) {
        m_min = std::min(m_min, value);
        m_max = std::max(m_max, value);
    }

    float step_size(bool logarithmic = false) const {
      if (m_max < m_min)
          return 0.0f;
      else if (logarithmic)
          return log(m_max / m_min) / ((float)Range_Colors.size() - 1.0f);
      else
          return (m_max - m_min) / ((float)Range_Colors.size() - 1.0f);
    }

    std::array<float, 3> get_color_at(float value, bool logarithmic = false) const {
        // std::lerp is available with c++20
        auto lerp = [](const std::array<float, 3>& a, const std::array<float, 3>& b, float t) {
            t = std::clamp(t, 0.0f, 1.0f);
            std::array<float, 3> ret;
            for (int i = 0; i < 3; ++i) {
                ret[i] = (1.0f - t) * a[i] + t * b[i];
            }
            return ret;
        };

        // Input value scaled to the colors range
        float global_t = 0.0f;
        const float step = step_size(logarithmic);
        value = std::clamp(value, m_min, m_max);
        if (step > 0.0f) {
            if (logarithmic)
                global_t = log(value / m_min) / step;
            else
                global_t = (value - m_min) / step;
        }

        const size_t color_max_idx = Range_Colors.size() - 1;

        // Compute the two colors just below (low) and above (high) the input value
        const size_t color_low_idx = std::clamp<size_t>(static_cast<size_t>(global_t), 0, color_max_idx);
        const size_t color_high_idx = std::clamp<size_t>(color_low_idx + 1, 0, color_max_idx);

        // Interpolate between the low and high colors to find exactly which color the input value should get
        return lerp(Range_Colors[color_low_idx], Range_Colors[color_high_idx], global_t - static_cast<float>(color_low_idx));
    }
};

Range width_range;
Range height_range;
Range speed_range;
Range fanspeed_range;
Range temperature_range;
Range volumetricrate_range;

void set_ranges(const std::vector<PathPoint>& path_points)
{
    width_range.reset();
    height_range.reset();
    speed_range.reset();
    fanspeed_range.reset();
    temperature_range.reset();
    volumetricrate_range.reset();

    for (size_t i = 0; i < path_points.size(); i++) {
        const PathPoint& p = path_points[i];
        if (p.is_extrude_move()) {
            if (config::extrusion_roles_visibility[globals::GCodeExtrusionRole::Custom] || !p.is_custom_gcode_move()) {
                width_range.update(p.width);
                volumetricrate_range.update(p.volumetricrate);
            }
            height_range.update(p.height);
            fanspeed_range.update(p.fanspeed);
            temperature_range.update(p.temperature);
        }
        if (config::travel_paths_visibility || p.is_extrude_move())
            speed_range.update(p.speed);
    }
}

struct BufferedPath
{
    GLuint                             positions_texture, positions_buffer;
#if ENABLE_ALTERNATE_SEGMENT_GEOMETRY
#if !ENABLE_PACKED_FLOATS
    GLuint                             height_width_texture, height_width_buffer;
#endif // !ENABLE_PACKED_FLOATS
#else
    GLuint                             height_width_angle_texture, height_width_angle_buffer;
#endif // ENABLE_ALTERNATE_SEGMENT_GEOMETRY
    GLuint                             color_texture, color_buffer;
    GLuint                             enabled_lines_texture, enabled_lines_buffer;
    size_t                             enabled_lines_count{ 0 };
    size_t                             total_points_count;
    bitset::BitSet<>                   valid_lines_bitset;
};

void updateEnabledLines(BufferedPath &path, const std::vector<PathPoint> &path_points) {
    std::vector<uint32_t> enabled_lines{};
    if (path.enabled_lines_count > 0)
        enabled_lines.reserve(path.enabled_lines_count);
    for (uint32_t i = sequential_range.get_current_min(); i < sequential_range.get_current_max(); i++) {
        const globals::GCodeExtrusionRole role = path_points[i].role_from_flags();

        if (!path.valid_lines_bitset[i]) continue;
        if (path_points[i].is_travel_move()) {
            if (!config::travel_paths_visibility) continue;
        }
        else if (path_points[i].is_option_move()) {
            if (!config::options_visibility[path_points[i].type_from_flags()]) continue;
        }
        else {
            if (!config::extrusion_roles_visibility[role]) continue;
        }
        enabled_lines.push_back(i);
    }

    globals::statistics.enabled_lines = enabled_lines.size();
    globals::statistics.enabled_lines_size = enabled_lines.size() * sizeof(uint32_t);

    glBindBuffer(GL_TEXTURE_BUFFER, path.enabled_lines_buffer);
    // buffer data to the buffer
    if (!enabled_lines.empty())
        glBufferData(GL_TEXTURE_BUFFER, enabled_lines.size() * sizeof(uint32_t), enabled_lines.data(), GL_STATIC_DRAW);
    else
        glBufferData(GL_TEXTURE_BUFFER, 0, nullptr, GL_STATIC_DRAW);

    path.enabled_lines_count = enabled_lines.size();
}

void updatePathColors(const BufferedPath &path, const std::vector<PathPoint> &path_points)
{
    auto select_color = [](const PathPoint& p) {
        const unsigned int role = (unsigned int)globals::extract_role_from_flags(p.flags);
        const globals::EMoveType type = globals::extract_type_from_flags(p.flags);
        if (type != globals::EMoveType::Travel && type != globals::EMoveType::Extrude)
            return Option_Colors.at(type);

        switch (config::visualization_type)
        {
        // feature type
        case 0:
        {
            assert((p.is_travel_move() && role < Travel_Colors.size()) || (p.is_extrude_move() && role < Extrusion_Role_Colors.size()));
            return p.is_travel_move() ? Travel_Colors[role] : Extrusion_Role_Colors[role];
        }
        // height
        case 1:
        {
            assert(!p.is_travel_move() || role < Travel_Colors.size());
            return p.is_travel_move() ? Travel_Colors[role] : height_range.get_color_at(p.height);
        }
        // width
        case 2: 
        {
            assert(!p.is_travel_move() || role < Travel_Colors.size());
            return p.is_travel_move() ? Travel_Colors[role] : width_range.get_color_at(p.width);
        }
        // speed
        case 3: { return speed_range.get_color_at(p.speed); }
        // fanspeed
        case 4: {
            assert(!p.is_travel_move() || role < Travel_Colors.size());
            return p.is_travel_move() ? Travel_Colors[role] : fanspeed_range.get_color_at(p.fanspeed);
        }
        // temperature
        case 5: {
            assert(!p.is_travel_move() || role < Travel_Colors.size());
            return p.is_travel_move() ? Travel_Colors[role] : temperature_range.get_color_at(p.temperature);
        }
        // volumetric rate
        case 6: {
            assert(!p.is_travel_move() || role < Travel_Colors.size());
            return p.is_travel_move() ? Travel_Colors[role] : volumetricrate_range.get_color_at(p.volumetricrate);
        }
        // tool
        case 9: {
            assert(p.extruderid < Tools_Colors.size());
            return Tools_Colors[p.extruderid];
        }
        // color print
        case 10: {
            return Tools_Colors[p.colorid % Tools_Colors.size()];
        }
        }

        return Dummy_Color;
    };

    auto encode_color = [select_color](const PathPoint& p) {
        const std::array<float, 3> color = select_color(p);
        const int r = (int)(255.0f * color[0]);
        const int g = (int)(255.0f * color[1]);
        const int b = (int)(255.0f * color[2]);
        const int i_color = r << 16 | g << 8 | b;
        return float(i_color);
    };

    std::vector<float> colors(path_points.size());
    for (size_t i = 0; i < path_points.size(); i++) {
        colors[i] = encode_color(path_points[i]);
    }

    globals::statistics.colors_size = colors.size() * sizeof(float);

    assert(path.color_buffer > 0);
    glBindBuffer(GL_TEXTURE_BUFFER, path.color_buffer);
    // buffer data to the path buffer
    glBufferData(GL_TEXTURE_BUFFER, colors.size() * sizeof(float), colors.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

BufferedPath bufferExtrusionPaths(const std::vector<PathPoint>& path_points) {
    BufferedPath result;

#if ENABLE_PACKED_FLOATS
    std::vector<glm::vec4> positions;
    std::vector<glm::vec2> height_width;
#else
    std::vector<glm::vec3> positions;
#if ENABLE_ALTERNATE_SEGMENT_GEOMETRY
    std::vector<glm::vec2> height_width;
#else
    std::vector<glm::vec3> height_width_angle;
#endif // ENABLE_ALTERNATE_SEGMENT_GEOMETRY
#endif // ENABLE_PACKED_FLOATS

    result.valid_lines_bitset = bitset::BitSet<>(path_points.size());
    result.valid_lines_bitset.setAll();

    for (size_t i = 0; i < path_points.size(); i++) {
        const PathPoint& p = path_points[i];
        const bool prev_line_valid = i > 0 && result.valid_lines_bitset[i - 1];
        const glm::vec3 prev_line = prev_line_valid ? (path_points[i].position - path_points[i - 1].position) : glm::vec3(0);

        const bool this_line_valid = i + 1 < path_points.size() &&
            path_points[i + 1].position != path_points[i].position &&
            path_points[i + 1].type_from_flags() == path_points[i].type_from_flags() &&
            path_points[i].type_from_flags() != globals::EMoveType::Seam;
        const glm::vec3 this_line = this_line_valid ? (path_points[i + 1].position - path_points[i].position) : glm::vec3(0);

        if (this_line_valid) {
            // there is a valid path between point i and i+1.
        }
        else {
            // the connection is invalid, there should be no line rendered, ever
            result.valid_lines_bitset.reset(i);
        }

#if ENABLE_PACKED_FLOATS
        const bool is_endpoint = !prev_line_valid || !this_line_valid;
        // store half height and half width
        const float h = 0.5f * p.height;
        const float w = 0.5f * p.width;
        // encode endpoints by using negative z
        positions.push_back({ p.position.x, p.position.y, is_endpoint ? -p.position.z : p.position.z, h });
        height_width.push_back({ h, w });
        const globals::EMoveType type = globals::extract_type_from_flags(p.flags);
        if (type == globals::EMoveType::Extrude ||
            type == globals::EMoveType::Wipe ||
            type == globals::EMoveType::Travel) {
            config::height_quantizer.update(h);
            config::width_quantizer.update(w);
        }
#endif // ENABLE_PACKED_FLOATS
#if ENABLE_ALTERNATE_SEGMENT_GEOMETRY
#if !ENABLE_PACKED_FLOATS
        const bool is_endpoint = !prev_line_valid || !this_line_valid;
        // encode endpoints by using negative z
        positions.push_back({ p.position.x, p.position.y, is_endpoint ? -p.position.z : p.position.z });
        height_width.push_back({ p.height, p.width });
#endif // !ENABLE_PACKED_FLOATS
#else
        const float angle = atan2(prev_line.x * this_line.y - prev_line.y * this_line.x, glm::dot(prev_line, this_line));
        height_width_angle.push_back({ p.height, p.width, angle });
#endif // ENABLE_ALTERNATE_SEGMENT_GEOMETRY
    }

#if ENABLE_PACKED_FLOATS
    // quantize heights and widths
    for (size_t i = 0; i < path_points.size(); i++) {
        const PathPoint& p = path_points[i];
        const globals::EMoveType type = globals::extract_type_from_flags(p.flags);
        if (type == globals::EMoveType::Extrude ||
            type == globals::EMoveType::Wipe ||
            type == globals::EMoveType::Travel) {
            const glm::vec2& hw = height_width[i];
            positions[i].w = config::height_quantizer.quantize(hw.x) * QuantizersResolution + config::width_quantizer.quantize(hw.y);
        }
    }
#endif // ENABLE_PACKED_FLOATS

    result.total_points_count = path_points.size();

#if ENABLE_PACKED_FLOATS
    globals::statistics.positions_size = positions.size() * sizeof(glm::vec4);
#else
    globals::statistics.positions_size = positions.size() * sizeof(glm::vec3);
#endif // ENABLE_PACKED_FLOATS
#if ENABLE_ALTERNATE_SEGMENT_GEOMETRY
#if !ENABLE_PACKED_FLOATS
    globals::statistics.height_width_size = height_width.size() * sizeof(glm::vec2);
#endif // !ENABLE_PACKED_FLOATS
#else
    globals::statistics.height_width_angle_size = height_width_angle.size() * sizeof(glm::vec3);
#endif // ENABLE_ALTERNATE_SEGMENT_GEOMETRY

    ///GCODE DATA
    glBindVertexArray(gcodeVAO);
    // Create a buffer object and bind it to the texture buffer
    glGenBuffers(1, &result.positions_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, result.positions_buffer);

    // buffer data to the path buffer
#if ENABLE_PACKED_FLOATS
    glBufferData(GL_TEXTURE_BUFFER, positions.size() * sizeof(glm::vec4), positions.data(), GL_STATIC_DRAW);
#else
    glBufferData(GL_TEXTURE_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
#endif // ENABLE_PACKED_FLOATS

    // Create and bind the path texture
    glGenTextures(1, &result.positions_texture);
    glBindTexture(GL_TEXTURE_BUFFER, result.positions_texture);

    // Attach the buffer object to the texture buffer
#if ENABLE_PACKED_FLOATS
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, result.positions_buffer);
#else
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, result.positions_buffer);
#endif // ENABLE_PACKED_FLOATS

    // Unbind the buffer object and the texture buffer
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

#if !ENABLE_PACKED_FLOATS
     // Create a buffer object and bind it to the texture buffer
#if ENABLE_ALTERNATE_SEGMENT_GEOMETRY
    glGenBuffers(1, &result.height_width_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, result.height_width_buffer);
#else
    glGenBuffers(1, &result.height_width_angle_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, result.height_width_angle_buffer);
#endif // ENABLE_ALTERNATE_SEGMENT_GEOMETRY

    // buffer data to the buffer
#if ENABLE_ALTERNATE_SEGMENT_GEOMETRY
    glBufferData(GL_TEXTURE_BUFFER, height_width.size() * sizeof(glm::vec2), height_width.data(), GL_STATIC_DRAW);

    // Create and bind the texture
    glGenTextures(1, &result.height_width_texture);
    glBindTexture(GL_TEXTURE_BUFFER, result.height_width_texture);

    // Attach the buffer object to the texture buffer
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32F, result.height_width_buffer);
#else
    glBufferData(GL_TEXTURE_BUFFER, height_width_angle.size() * sizeof(glm::vec3), height_width_angle.data(), GL_STATIC_DRAW);

    // Create and bind the texture
    glGenTextures(1, &result.height_width_angle_texture);
    glBindTexture(GL_TEXTURE_BUFFER, result.height_width_angle_texture);

    // Attach the buffer object to the texture buffer
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, result.height_width_angle_buffer);
#endif // ENABLE_ALTERNATE_SEGMENT_GEOMETRY
#endif // !ENABLE_PACKED_FLOATS

    // Unbind the buffer object and the texture buffer
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

    //COLOR BUFFER
    // Create a buffer object and bind it to the texture buffer
    glGenBuffers(1, &result.color_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, result.color_buffer);

    // Create and bind the texture
    glGenTextures(1, &result.color_texture);
    glBindTexture(GL_TEXTURE_BUFFER, result.color_texture);

    // Attach the buffer object to the texture buffer
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, result.color_buffer);

    // Unbind the buffer object and the texture buffer
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

    // ENABLED LINES BUFFER
    // Create a buffer object and bind it to the texture buffer
    glGenBuffers(1, &result.enabled_lines_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, result.enabled_lines_buffer);

    // Create and bind the texture
    glGenTextures(1, &result.enabled_lines_texture);
    glBindTexture(GL_TEXTURE_BUFFER, result.enabled_lines_texture);

    // Attach the buffer object to the texture buffer
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, result.enabled_lines_buffer);

    // buffer enabled lines
    updateEnabledLines(result, path_points);

    // Unbind the buffer object and the texture buffer
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

    glBindVertexArray(0);

    return result;
}

BufferedPath generateTestingPathPoints()
{
    std::vector<PathPoint>                pathPoints;
    std::mt19937                          rng(std::random_device{}()); // Random number generator
    std::uniform_real_distribution<float> sizeDist(0.05f, 2.0f);        // Random size distribution
    std::uniform_real_distribution<float> diffDistance(-50.0f, 50.0f);   // Random size distribution

    glm::vec3 last_point = glm::vec3(diffDistance(rng), diffDistance(rng), 0); 
    for (int i = 0; i < 50; ++i) {
        PathPoint point;
        point.position = last_point + glm::vec3(diffDistance(rng), diffDistance(rng), diffDistance(rng));
        point.encode_flags(1 + rand() % 12, 10); // role = Random extrusion role, type = Extrude
        point.height = sizeDist(rng);            // Random height between 1.0 and 5.0
        point.width  = point.height;//2.0*sizeDist(rng);        // Random width between 1.0 and 5.0
        last_point = point.position;
        pathPoints.push_back(point);
    }

   return bufferExtrusionPaths(pathPoints);
}

void init()
{
    glGenVertexArrays(1, &gcodeVAO);
    glBindVertexArray(gcodeVAO);

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    checkGl();

    globals::statistics.vertex_data_size = vertex_data.size() * sizeof(uint8_t);

    glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(uint8_t), vertex_data.data(), GL_STATIC_DRAW);

    // vertex attributes - id:
    glEnableVertexAttribArray(vid_loc);
    glVertexAttribIPointer(vid_loc, 1, GL_UNSIGNED_BYTE, sizeof(uint8_t), (void*)0);
    checkGl();
    glBindVertexArray(0);
}


} // namespace gcode

#endif /* GCODE_H_ */
