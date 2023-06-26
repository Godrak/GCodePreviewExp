/*
 * gcode.h
 *
 *  Created on: Jun 9, 2019
 *      Author: Pavel Mikus
 *		mail: pavel.mikus@eyen.se
 */

#ifndef GCODE_H_
#define GCODE_H_

#include "glad/glad.h"

#include "globals.h"
#include "camera.h"

#include <cstddef>
#include <future>
#include <glm/fwd.hpp>
#include <vector>
#include <random>
#include <iostream>
#include <algorithm>
#include <array>

namespace gcode {

static unsigned int extract_role_from_flags(unsigned int flags) { return flags & 0xFF; }
static unsigned int extract_type_from_flags(unsigned int flags) { return (flags >> 8) & 0xFF; }

GLuint gcodeVAO, vertexBuffer;
GLuint visibilityFramebuffer, instanceIdsTexture, depthTexture;
GLuint quadVAO;

GLuint pathSSBObindPoint = 5;

GLint vid_loc = 0;

size_t vertex_data_size = 18;
int vertex_data[] = {  
        0, 1, 2,  // left side of box (for common depiction of a box, where left, right and top sides are visible)
        0, 2, 3,
        0, 3, 4,  // right side of box    
        0, 4, 5, 
        0, 5, 6, // top side of box 
        0, 6, 1 };

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
    unsigned int role_from_flags() const { return extract_role_from_flags(flags); }
    unsigned int type_from_flags() const { return extract_type_from_flags(flags); }
    bool is_travel_move() const  { return extract_type_from_flags(flags) == 8; }
    bool is_extrude_move() const { return extract_type_from_flags(flags) == 10; }
};

struct BufferedPath
{
    GLuint positions_texture, positions_buffer;
    GLuint height_width_color_texture, height_width_color_buffer;
    GLuint enabled_segments_texture, enabled_segments_buffer;
    size_t enabled_segments_count = 0;
    GLuint visible_segments_texture;
    std::pair<GLuint, GLuint> visible_segments_doublebuffer;
    std::pair<size_t, size_t> visible_segments_counts = {0,0};
    GLuint visibility_pixel_buffer;
    glm::ivec2 visibility_texture_size; 
    
    std::vector<std::vector<glm::uint32>> visibility_vectors = std::vector<std::vector<glm::uint32>>(10);
    std::vector<glm::uint32> visibility_vector{};
    size_t current_visibility_index = 0;
    std::future<void> filtering_future{};
    GLsync rendering_sync_fence;
    GLsync buffering_sync_fence;
};

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
            width_range.update(p.width);
            height_range.update(p.height);
            fanspeed_range.update(p.fanspeed);
            temperature_range.update(p.temperature);
            volumetricrate_range.update(p.volumetricrate);
        }
        if (config::use_travel_moves_data || p.is_extrude_move())
            speed_range.update(p.speed);
    }
}

void updatePathColors(const BufferedPath& path, const std::vector<PathPoint>& path_points)
{
    auto select_color = [](const PathPoint& p) {
        static const std::array<float, 3> error_color = { 0.5f, 0.5f, 0.5f };
        const unsigned int role = extract_role_from_flags(p.flags);
        const unsigned int type = extract_type_from_flags(p.flags);
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

        return error_color;
    };

    auto format_color = [&](const PathPoint& p) {
        const std::array<float, 3> color = select_color(p);
        const int r = (int)(255.0f * color[0]);
        const int g = (int)(255.0f * color[1]);
        const int b = (int)(255.0f * color[2]);
        const int i_color = r << 16 | g << 8 | b;
        return float(i_color);
    };

    assert(path.height_width_color_buffer > 0);
    glBindBuffer(GL_TEXTURE_BUFFER, path.height_width_color_buffer);

    const char* ptr = (const char*)glMapBuffer(GL_TEXTURE_BUFFER, GL_WRITE_ONLY);

    assert(ptr != nullptr);

    for (size_t i = 0; i < path_points.size(); i++) {
        const float color = format_color(path_points[i]);
        const size_t offset = (i * 3 + 2) * sizeof(float);
        memcpy((void*)(ptr + offset), (const void*)&color, sizeof(color));
    }

    glUnmapBuffer(GL_TEXTURE_BUFFER);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

BufferedPath bufferExtrusionPaths(const std::vector<PathPoint>& path_points) {
    BufferedPath result;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> height_width_color;
    std::vector<glm::uint32> enabled_segments;

    for (size_t i = 0; i < path_points.size(); i++) {
        if (i + 1 < path_points.size() && path_points[i + 1].position != path_points[i].position)
            enabled_segments.push_back((glm::uint32)positions.size());

        const PathPoint& p = path_points[i];
        positions.push_back({ p.position });
        const float height = p.is_travel_move() ? 0.1f : p.height;
        const float width  = p.is_travel_move() ? 0.1f : p.width;

        height_width_color.push_back({ height, width, 0.0f }); // color will be set later with updatePathColors()
    }

    result.enabled_segments_count = enabled_segments.size();

    glBindVertexArray(gcodeVAO);

    // Create a buffer object and bind it to the texture buffer
    glGenBuffers(1, &result.positions_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, result.positions_buffer);

    // buffer data to the path buffer
    glBufferData(GL_TEXTURE_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);

    // Create and bind the path texture
    glGenTextures(1, &result.positions_texture);
    glBindTexture(GL_TEXTURE_BUFFER, result.positions_texture);

    // Attach the buffer object to the texture buffer
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, result.positions_buffer);

    // Unbind the buffer object and the texture buffer
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

     // Create a buffer object and bind it to the texture buffer
    glGenBuffers(1, &result.height_width_color_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, result.height_width_color_buffer);

    // buffer data to the path buffer
    glBufferData(GL_TEXTURE_BUFFER, height_width_color.size() * sizeof(glm::vec3), height_width_color.data(), GL_DYNAMIC_DRAW);

    // Create and bind the path texture
    glGenTextures(1, &result.height_width_color_texture);
    glBindTexture(GL_TEXTURE_BUFFER, result.height_width_color_texture);

    // Attach the buffer object to the texture buffer
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, result.height_width_color_buffer);

    // Unbind the buffer object and the texture buffer
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

   // Create a buffer object and bind it to the texture buffer
    glGenBuffers(1, &result.enabled_segments_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, result.enabled_segments_buffer);

    // buffer data to the path buffer
    glBufferData(GL_TEXTURE_BUFFER, enabled_segments.size() * sizeof(glm::uint32), enabled_segments.data(), GL_STATIC_DRAW);

    // Create and bind the path texture
    glGenTextures(1, &result.enabled_segments_texture);
    glBindTexture(GL_TEXTURE_BUFFER, result.enabled_segments_texture);

    // Attach the buffer object to the texture buffer
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, result.enabled_segments_buffer);

    // Unbind the buffer object and the texture buffer
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

    // Create a buffer object and bind it to the texture buffer
    glGenBuffers(1, &result.visible_segments_doublebuffer.first);
    glGenBuffers(1, &result.visible_segments_doublebuffer.second);
    glGenTextures(1, &result.enabled_segments_texture);
    glGenBuffers(1, &result.visibility_pixel_buffer);

    result.rendering_sync_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    result.buffering_sync_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

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

void recreateVisibilityBufferOnResolutionChange()
{
    // Delete existing textures and framebuffer
    glDeleteTextures(1, &instanceIdsTexture);
    glDeleteTextures(1, &depthTexture);
    glDeleteFramebuffers(1, &visibilityFramebuffer);

    glGenFramebuffers(1, &visibilityFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, visibilityFramebuffer);

    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &instanceIdsTexture);
    glBindTexture(GL_TEXTURE_2D, instanceIdsTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, globals::visibilityResolution.x, globals::visibilityResolution.y, 0, GL_RED_INTEGER, GL_UNSIGNED_INT,
                 nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, instanceIdsTexture, 0);

    checkGl();

    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, globals::visibilityResolution.x, globals::visibilityResolution.y, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    checkGl();
    checkGLCall(glCheckFramebufferStatus(visibilityFramebuffer));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void init()
{
    glGenVertexArrays(1, &gcodeVAO);
    glBindVertexArray(gcodeVAO);

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    checkGl();

    glBufferData(GL_ARRAY_BUFFER, vertex_data_size * sizeof(int), vertex_data, GL_STATIC_DRAW);

    // vertex attributes - id:
    glEnableVertexAttribArray(vid_loc);
    glVertexAttribIPointer(vid_loc, 1, GL_INT, sizeof(int), (void *)0);
    checkGl();

    glBindVertexArray(0);

    glGenFramebuffers(1, &visibilityFramebuffer);
    glGenTextures(1, &instanceIdsTexture);
  	glGenTextures(1, &depthTexture);
    recreateVisibilityBufferOnResolutionChange();

    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);

    checkGl();

    glBindVertexArray(0);
}


} // namespace gcode

#endif /* GCODE_H_ */
