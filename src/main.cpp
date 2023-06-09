// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

#include <fstream>

#include "camera.h"
#include "gcode.h"
#include "shaders.h"
#include <glm/gtc/type_ptr.hpp>

namespace glfwContext {
GLFWwindow* window{nullptr};
int vsync{ 1 };
char forth_back{ ' ' };
char left_right{ ' ' };
char up_down{ ' ' };
char top_view{ ' ' };
double last_xpos{ 0.0 };
double last_ypos{ 0.0 };

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch (action)
    {
    case GLFW_PRESS: {
        switch (key)
        {
        case GLFW_KEY_ESCAPE: {
            glfwSetWindowShouldClose(window, 1);
            break;
        }
        case GLFW_KEY_F2: {
            config::geometryMode = 1 - config::geometryMode;
            break;
        }
        case GLFW_KEY_F3: {
            vsync = 1 - vsync;
            std::cout << "vsync: " << vsync << std::endl;
            glfwSwapInterval(vsync);
            break;
        }
        case GLFW_KEY_F4: {
            config::with_visibility_pass = 1 - config::with_visibility_pass;
            std::cout << "with_visibility_pass: " << config::with_visibility_pass << std::endl;
            break;
        }
        case GLFW_KEY_R: {
            config::percentage_to_show += 0.005f;
            config::percentage_to_show = fmin(1.0f, config::percentage_to_show);
            break;
        }
        case GLFW_KEY_F: {
            config::percentage_to_show -= 0.005f;
            config::percentage_to_show = fmax(0.0f, config::percentage_to_show);
            break;
        }
        case GLFW_KEY_W: {
            forth_back = ('w');
            break;
        }
        case GLFW_KEY_S: {
            forth_back = ('s');
            break;
        }
        case GLFW_KEY_A: {
            left_right = ('a');
            break;
        }
        case GLFW_KEY_D: {
            left_right = ('d');
            break;
        }
        case GLFW_KEY_Q: {
            up_down = ('q');
            break;
        }
        case GLFW_KEY_Z: {
            up_down = ('z');
            break;
        }
        case GLFW_KEY_T: {
            top_view = ('t');
            break;
        }
        case GLFW_KEY_KP_ADD: {
            camera::stepSize *= 2.0f;
            break;
        }
        case GLFW_KEY_KP_SUBTRACT: {
            camera::stepSize *= 0.5f;
            break;
        }
        }
        break;
    }
    case GLFW_RELEASE: {
        switch (key) {
        case GLFW_KEY_W:
        case GLFW_KEY_S: {
            forth_back = ' ';
            break;
        }
        case GLFW_KEY_A:
        case GLFW_KEY_D: {
            left_right = ' ';
            break;
        }
        case GLFW_KEY_Q:
        case GLFW_KEY_Z: {
            up_down = ' ';
            break;
        }
        case GLFW_KEY_T: {
            top_view = 't';
            break;
        }
        }
        break;
    }
    }
}

static void glfw_cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    glm::vec2 offset;
    offset[0] = (float)(-xpos + last_xpos);
    offset[1] = (float)(ypos - last_ypos);
    offset *= 0.001f * camera::rotationSpeed;
    camera::moveCamera(offset);

    last_xpos = xpos;
    last_ypos = ypos;
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
    file.read(reinterpret_cast<char*>(&size), sizeof(size));
    std::cout << "SIZE IS: " << size << std::endl;
    pathPoints.resize(size);

    // Read each PathPoint object from the file
    for (auto& point : pathPoints) {
        file.read(reinterpret_cast<char*>(&point), sizeof(point));
    }

    file.close();

    return pathPoints;
}

static void show_fps()
{
    ImGui::SetNextWindowPos({ 0.0f, 0.0f }, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("##fps", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
    ImGui::Text("FPS %d", (unsigned int)ImGui::GetCurrentContext()->IO.Framerate);
    ImGui::End();
    ImGui::PopStyleVar();
}

namespace rendering {
    float lastTime;
    float currentTime;
    glm::vec3 lastCameraPosition = camera::position;

    void switchConfiguration() {
        if (config::geometryMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); checkGl();
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); checkGl();
        }
    }

    void render(const gcode::BufferedPath& path) {
        currentTime = float(glfwGetTime()); checkGl();
        auto delta = currentTime - lastTime;
        lastTime = currentTime;
        glfwGetFramebufferSize(glfwContext::window, &globals::screenResolution.x, &globals::screenResolution.y); checkGl();

        glViewport(0, 0, globals::screenResolution.x, globals::screenResolution.y); checkGl();

        camera::moveCamera(glfwContext::forth_back); checkGl();
        camera::moveCamera(glfwContext::up_down); checkGl();
        camera::moveCamera(glfwContext::left_right); checkGl();
        camera::moveCamera(glfwContext::top_view); checkGl();

        if (config::updateCameraPosition)
            lastCameraPosition = camera::position;

        glm::mat4x4 view_projection = glm::mat4x4(1.0); checkGl();
        camera::applyViewTransform(view_projection);
        camera::applyProjectionTransform(view_projection);

        if (config::with_visibility_pass) {
            // Visibility pass. This will store visible points IDs into the visibility framebuffer texture
            // first, reset the visibility values
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, path.visibility_buffer);
            glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32I, GL_RED_INTEGER, GL_INT, 0);

            // bind visibility framebuffer, clear it and render all lines
            glBindFramebuffer(GL_FRAMEBUFFER, gcode::visibilityFrameBuffer);
            checkGl();
            glEnable(GL_DEPTH_TEST);
            glClearColor(0.0, 0.0, 0.0, 0.0); // This will be interpreted as one integer, probably from the first component. I am putting it
                            // here anyway, to make it clear
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            checkGl();
            glViewport(0, 0, globals::visibilityResolution.x, globals::visibilityResolution.y);

            // except for the different resolution and shader program, this render pass is same as the final GCode render.
            // what happens here is that all boxes of all lines are rendered, but only the visible ones will have their ids written into the framebuffer
            glBindVertexArray(gcode::gcodeVAO);
            glUseProgram(shaderProgram::visibility_program);
            checkGl();

            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, path.path_buffer); // Bind the SSBO to the indexed buffer binding point 0
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, path.visibility_buffer);

            glUniformMatrix4fv(globals::vp_location, 1, GL_FALSE, glm::value_ptr(view_projection));
            glUniform3fv(globals::camera_position_location, 1, glm::value_ptr(lastCameraPosition));
            // this tells the vertex shader to ignore visiblity values and render all lines instead
            glUniform1i(globals::visibility_pass_location, true);
            checkGl();
            glDrawArraysInstanced(GL_TRIANGLES, 0, gcode::vertex_data_size,
              std::max(size_t(2), size_t((path.point_count - 1) * config::percentage_to_show)));
            checkGl();

            glUseProgram(0);
            glBindVertexArray(0);

            // now with the visibility texture filled with ids, we need to set the values in the visibility buffer to true for corresponding values
            glBindVertexArray(gcode::quadVAO);
            glUseProgram(shaderProgram::quad_program);

            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, path.visibility_buffer);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, gcode::instanceIdsTexture);
            glBindImageTexture(1 /*UNIT*/, gcode::instanceIdsTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32I);

            checkGl();

            glDrawArrays(GL_TRIANGLES, 0, 6);

            checkGl();
            glUseProgram(0);
            glBindVertexArray(0);
        }

        // Now render only the visible lines, with the expensive frag shader
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        checkGl();
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.6, 0.6, 0.6, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        checkGl();
        glViewport(0, 0, globals::screenResolution.x, globals::screenResolution.y);
        checkGl();

        glBindVertexArray(gcode::gcodeVAO);
        glUseProgram(shaderProgram::gcode_program);
        checkGl();

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, path.path_buffer); // Bind the SSBO to the indexed buffer binding point 0
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, path.visibility_buffer);

        glUniformMatrix4fv(globals::vp_location, 1, GL_FALSE, glm::value_ptr(view_projection));
        glUniform3fv(globals::camera_position_location, 1, glm::value_ptr(lastCameraPosition));
        glUniform1i(globals::visibility_pass_location, false);
        checkGl();
        glDrawArraysInstanced(GL_TRIANGLES, 0, gcode::vertex_data_size,
            std::max(size_t(2), size_t((path.point_count - 1) * config::percentage_to_show)));
        checkGl();

        glUseProgram(0);
        glBindVertexArray(0);
    }

    void setup() {
        shaderProgram::createGCodeProgram();
        shaderProgram::createVisibilityProgram();
        shaderProgram::createQuadProgram();
        gcode::init();
        checkGl();
    }
} // namespace rendering

// Main code
int main(int argc, char* argv[])
{
    // Check if a filename argument is provided
    if (argc < 2) {
        std::cout << "Please provide a filename as an argument." << std::endl;
        return 1;
    }

    // Read the filename from argv[1]
    const std::string filename = argv[1];

    auto points = readPathPoints(filename);

    glfwSetErrorCallback(glfwContext::glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    glfwContext::window = window;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    glfwSetKeyCallback(window, glfwContext::glfw_key_callback);
    glfwSetCursorPosCallback(window, glfwContext::glfw_cursor_position_callback);

    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);


    rendering::lastTime = float(glfwGetTime());
    rendering::currentTime = float(glfwGetTime());

    rendering::setup();

    // gcode::BufferedPath path = gcode::generateTestingPathPoints();
    gcode::BufferedPath path = gcode::bufferExtrusionPaths(points);

    std::cout << "PATHS BUFFERED" << std::endl;

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

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
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        rendering::switchConfiguration();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        show_fps();
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
