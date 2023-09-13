#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include <iostream>
#include <algorithm>
#include <string>
#include <map>
#include "bitset"

namespace globals {
		glm::ivec2 screenResolution = {512,512};

		enum class GCodeExtrusionRole : uint8_t
		{
				None,
				Perimeter,
				ExternalPerimeter,
				OverhangPerimeter,
				InternalInfill,
				SolidInfill,
				TopSolidInfill,
				Ironing,
				BridgeInfill,
				GapFill,
				Skirt,
				SupportMaterial,
				SupportMaterialInterface,
				WipeTower,
				// Custom (user defined) G-code block, for example start / end G-code.
				Custom,
				// Stopper to count number of enums.
				Count
		};

		std::string gcode_extrusion_role_to_string(GCodeExtrusionRole role)
		{
				switch (role) {
				case GCodeExtrusionRole::None:										 return "Unknown";
				case GCodeExtrusionRole::Perimeter:								 return "Perimeter";
				case GCodeExtrusionRole::ExternalPerimeter:				 return "External perimeter";
				case GCodeExtrusionRole::OverhangPerimeter:				 return "Overhang perimeter";
				case GCodeExtrusionRole::InternalInfill:					 return "Internal infill";
				case GCodeExtrusionRole::SolidInfill:							 return "Solid infill";
				case GCodeExtrusionRole::TopSolidInfill:					 return "Top solid infill";
				case GCodeExtrusionRole::Ironing:									 return "Ironing";
				case GCodeExtrusionRole::BridgeInfill:						 return "Bridge infill";
				case GCodeExtrusionRole::GapFill:									 return "Gap fill";
				case GCodeExtrusionRole::Skirt:										 return "Skirt/Brim";
				case GCodeExtrusionRole::SupportMaterial:					 return "Support material";
				case GCodeExtrusionRole::SupportMaterialInterface: return "Support material interface";
				case GCodeExtrusionRole::WipeTower:								 return "Wipe tower";
				case GCodeExtrusionRole::Custom:									 return "Custom";
				default:																				   assert(false);
				}
				return std::string();
		}

		GCodeExtrusionRole extract_role_from_flags(unsigned int flags) {
				const unsigned int value = flags & 0xFF;
				assert(value < (unsigned int)GCodeExtrusionRole::Count);
				return (GCodeExtrusionRole)value;
		}

		enum class EMoveType : uint8_t
		{
				Noop,
				Retract,
				Unretract,
				Seam,
				Tool_change,
				Color_change,
				Pause_print,
				Custom_GCode,
				Travel,
				Wipe,
				Extrude,
				// Stopper to count number of enums.
				Count
		};

		std::string gcode_move_type_to_string(EMoveType type)
		{
				switch (type) {
				case EMoveType::Noop:					return "Noop";
				case EMoveType::Retract:			return "Retract";
				case EMoveType::Unretract:	  return "Unretract";
				case EMoveType::Seam:				  return "Seam";
				case EMoveType::Tool_change:	return "Tool change";
				case EMoveType::Color_change: return "Color change";
				case EMoveType::Pause_print:	return "Pause print";
				case EMoveType::Custom_GCode:	return "Custom GCode";
				case EMoveType::Travel:				return "Travel";
				case EMoveType::Wipe:					return "Wipe";
				case EMoveType::Extrude:			return "Extrude";
				default:											assert(false);
				}
				return std::string();
		}

		EMoveType extract_type_from_flags(unsigned int flags) {
				const unsigned int value = (flags >> 8) & 0xFF;
				assert(value < (unsigned int)EMoveType::Count);
				return (EMoveType)value;
		}

		struct Statistics
		{
				size_t total_moves{ 0 };
				size_t moves_size{ 0 };
				size_t enabled_lines{ 0 };
				size_t enabled_lines_size{ 0 };
				size_t colors_size{ 0 };
				size_t positions_size{ 0 };
				size_t height_width_angle_size{ 0 };
				size_t vertex_data_size{ 0 };
				size_t buffers_size() const { return enabled_lines_size + colors_size + positions_size + height_width_angle_size + vertex_data_size; }
		};

		Statistics statistics;

		std::pair<float, std::string> format_memsize(size_t size)
		{
				static const size_t KILO = 1024;
				static const size_t MEGA = 1024 * 1024;
				static const size_t GIGA = 1024 * 1024 * 1024;
				if (size < KILO)
						return { float(size), "bytes" };
				else if (size < MEGA)
						return { float(size) / float(KILO), "KB" };
				else if (size < GIGA)
						return { float(size) / float(MEGA), "MB" };
				else
						return { float(size) / float(GIGA), "GB" };
		}
} // namespace globals

namespace config {
bool updateCameraPosition = true;
bool geometryMode = false;
bool vsync = true;
int visualization_type = 0;
bool ranges_update_required = true;
bool color_update_required = true;
bool camera_center_required = true;
bool use_travel_moves_data = true;
bool window_minimized = false;
bool force_full_model_render = false;
std::map<globals::GCodeExtrusionRole, bool> extrusion_roles_visibility{{
		{ globals::GCodeExtrusionRole::None, true },
		{ globals::GCodeExtrusionRole::Perimeter, true },
		{ globals::GCodeExtrusionRole::ExternalPerimeter, true },
		{ globals::GCodeExtrusionRole::OverhangPerimeter, true },
		{ globals::GCodeExtrusionRole::InternalInfill, true },
		{ globals::GCodeExtrusionRole::SolidInfill, true },
		{ globals::GCodeExtrusionRole::TopSolidInfill, true },
		{ globals::GCodeExtrusionRole::Ironing, true },
		{ globals::GCodeExtrusionRole::BridgeInfill, true },
		{ globals::GCodeExtrusionRole::GapFill, true },
		{ globals::GCodeExtrusionRole::Skirt, true },
		{ globals::GCodeExtrusionRole::SupportMaterial, true },
		{ globals::GCodeExtrusionRole::SupportMaterialInterface, true },
		{ globals::GCodeExtrusionRole::WipeTower, true },
		{ globals::GCodeExtrusionRole::Custom, true }
}};
std::map<globals::EMoveType, bool> options_visibility{{
		{ globals::EMoveType::Retract, true },
		{ globals::EMoveType::Unretract, true },
		{ globals::EMoveType::Seam, true },
		{ globals::EMoveType::Tool_change, true },
		{ globals::EMoveType::Color_change, true },
		{ globals::EMoveType::Pause_print, true },
		{ globals::EMoveType::Custom_GCode, true },
		{ globals::EMoveType::Wipe, true }
}};
bool travel_paths_visibility{ true };
bool enabled_paths_update_required = true;


} // namespace config

class SequentialRange
{
		size_t m_global_min{ 0 };
		size_t m_global_max{ 0 };
		size_t m_current_min{ 0 };
		size_t m_current_max{ 0 };

public:
		void set_current_min(size_t min) {
				assert(min > 0);
				m_current_min = std::clamp(min - 1, m_global_min, m_global_max);
				if (m_current_max < m_current_min)
						m_current_max = m_current_min;
		}
		void set_current_max(size_t max) {
				assert(max > 0);
				m_current_max = std::clamp(max - 1, m_global_min, m_global_max);
				if (m_current_min > m_current_max)
						m_current_min = m_current_max;
		}
		void set_global_max(size_t max) {
				assert(max > 0);
				m_global_max = max - 1;
				assert(m_global_min <= m_global_max);
		}

		size_t get_current_size() const { return m_current_max - m_current_min; }

		size_t get_global_min() const { return m_global_min + 1; }
		size_t get_global_max() const { return m_global_max + 1; }
		size_t get_current_min() const { return m_current_min + 1; }
		size_t get_current_max() const { return m_current_max + 1; }

		void increase_current_min(size_t value) {
				m_current_min = (value + m_current_min > m_global_max) ? m_global_max : m_current_min + value;
				if (m_current_max < m_current_min)
						m_current_max = m_current_min;
		}
		void decrease_current_min(size_t value) {
				m_current_min = (value > m_current_min - m_global_min) ? m_global_min : m_current_min - value;
		}
		void increase_current_max(size_t value) {
				m_current_max = (value + m_current_max > m_global_max) ? m_global_max : m_current_max + value;
		}
		void decrease_current_max(size_t value) {
				m_current_max = (value > m_current_max - m_global_min) ? m_global_min : m_current_max - value;
				if (m_current_min > m_current_max)
						m_current_min = m_current_max;
		}
};

SequentialRange sequential_range;

static bool check_opengl() {
	GLenum err = glGetError();
	if (err == GL_NO_ERROR)
		return true;
	std::cout << "OpenGL message: " << (int) err << std::endl;
	return false;
}

static bool checkGLCall(GLenum err) {
	if (err == GL_NO_ERROR)
		return true;
	std::cout << "OpenGL message: " << (int) err << std::endl;
	return false;
}

static void checkGl() {
#ifdef DEBUG
	if (!check_opengl()) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
#endif
}

#endif /* GLOBALS_H_ */
