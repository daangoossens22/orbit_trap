#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "shader.h"
#include "camera.h"

#include <iostream>
#include <vector>

// save and convert the image from the opengl buffer to some image format
#include <FreeImage.h>

// variables shared with shader
const int MAX_ORBIT_POINTS = 20;
const int MAX_PALETTE_COLORS = 20;

const std::string fractal_vertex_path = "src/fractal_shaders/shader.vert";
const std::string fractal_fragment_path = "src/fractal_shaders/shader.frag";

const char* image_save_path = "output_image.png";

const int width = 1280;
const int heigth = 720;

Camera camera = Camera((float)width, (float)heigth);

static void zoom_by_scrolling_callback(GLFWwindow* window, double xoffset, double yoffset);
static void drag_translation_callback(GLFWwindow* window, int button, int action, int mods);
void get_cursor_pos(GLFWwindow* window, double& xpos, double& ypos);

GLFWwindow* setup();
static void glfw_error_callback(int error, const char* description);

int change_orbit_point_pos = 0;
int num_orbit_points = 1;
float orbit_points[2 * MAX_ORBIT_POINTS] = { 0.0f };

ImGuiMouseCursor_ current_cursor = ImGuiMouseCursor_Arrow;

// TODO
// 1. center camera button / function / imgui
// 2. save opengl frame buffer to png (don't overwrite images, make framebuffer 1080p when saving the image, use multiple rays per pixel for saving the image (antialiasing))
// 3. clean up imgui window (with trees??) and make orbit point selection a list (and color the selected point a different color)
// 4. orbit lines
// 5. orbit boxes
// 6. orbit circles
// 7. orbit triangles
// 8. orbit hexagons
// 9. different fractals (julia sets?, etc.)
// 10. higher precision mandelbrot without using doubles
// 11. clean up codebase / refine code functionality
// 12. be able to drag orbit points instead of only pressing continuously


int main(int, char**)
{
    GLFWwindow* window = setup();

    glfwSetScrollCallback(window, zoom_by_scrolling_callback);
    glfwSetMouseButtonCallback(window, drag_translation_callback);

    Shader shader = Shader(fractal_vertex_path, fractal_fragment_path);

    // fill screen with quad (rectangle/2 triangles)
    float vertices[] =
    {
         1.0f,  1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f
    };
    unsigned int indices[] =
    {
        0, 1, 3,
        1, 2, 3
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Our state
    bool show_demo_window = false;
    int num_palette_colors = 4;
    std::vector<ImVec4> palette;
    palette.push_back(ImVec4(0.92f, 0.86f, 0.86f, 1.0f));
    palette.push_back(ImVec4(0.55f, 0.19f, 0.19f, 1.0f));
    palette.push_back(ImVec4(0.94f, 0.87f, 0.47f, 1.0f));
    palette.push_back(ImVec4(0.09f, 0.36f, 0.55f, 1.0f));
    palette.push_back(ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
    palette.push_back(ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
    palette.push_back(ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
    palette.push_back(ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
    float coloring_shift = 1.1111f;

    bool save_image = false;

    bool draw_orbit_points = true;
    ImVec4 orbit_point_colors[2] = { ImVec4(1.0f, 0.5f, 0.0f, 1.0f), ImVec4(0.0f, 1.0f, 0.8f, 1.0f) };

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        double xpos, ypos;
        get_cursor_pos(window, xpos, ypos);
        camera.drag_update(xpos, ypos);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetMouseCursor(current_cursor);

        // ImGui windows
        {
            ImGui::Begin("Hello, world!");

            ImGui::Checkbox("Demo Window", &show_demo_window);
            ImGui::SliderInt("num palette colors", &num_palette_colors, 1, 8);
            for (int i = 0; i < num_palette_colors; ++i)
            {
                std::string label_name = "palette color " + std::to_string(i);
                ImGui::ColorEdit3(label_name.c_str(), (float*)&palette[i]);
            }

            ImGui::SliderFloat("coloring shift", &coloring_shift, 0.0f, 6.3f);

            ImGui::Checkbox("draw orbit points", &draw_orbit_points);
            ImGui::ColorEdit3("orbit_point_color", (float*)&orbit_point_colors[0]);
            ImGui::ColorEdit3("selected orbit_point_color", (float*)&orbit_point_colors[1]);
            ImGui::Text("set the orbit point position with the right mouse button");
            ImGui::SliderInt("num orbit points", &num_orbit_points, 0, MAX_ORBIT_POINTS);
            auto label_generator_orbit_points = [](int orbit_point) { return orbit_point ? "orbit point " + std::to_string(orbit_point) : "no orbit point selected"; };
            if (ImGui::BeginCombo("change orbit point", label_generator_orbit_points(change_orbit_point_pos).c_str(), 0))
            {
                for (int i = 0; i < num_orbit_points + 1; i++)
                {
                    const bool is_selected = (change_orbit_point_pos == i);
                    if (ImGui::Selectable(label_generator_orbit_points(i).c_str(), is_selected))
                    {
                        change_orbit_point_pos = i;
                    }
                    if (is_selected) { ImGui::SetItemDefaultFocus(); }
                }
                ImGui::EndCombo();
            }

            ImGui::Checkbox("save image", &save_image);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            ImGui::End();
        }
        if (show_demo_window) { ImGui::ShowDemoWindow(&show_demo_window); }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0, 0.6, 1.0, 1.0); // cyany color
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();

        // set uniform variables
        camera.move_camera_matrix_to_gpu(shader.ID);

        unsigned int sin_buf = glGetUniformLocation(shader.ID, "coloring_shift");
        glUniform1f(sin_buf, coloring_shift);

        glUniform1i(glGetUniformLocation(shader.ID, "num_palette_colors"), num_palette_colors);
        glUniform4fv(glGetUniformLocation(shader.ID, "palette"), num_palette_colors, (float*)&palette[0]);

        glUniform1i(glGetUniformLocation(shader.ID, "draw_orbit_points"), (int)draw_orbit_points);
        glUniform4fv(glGetUniformLocation(shader.ID, "orbit_point_colors"), 2, (float*)&orbit_point_colors[0]);
        glUniform1i(glGetUniformLocation(shader.ID, "orbit_point_selected"), change_orbit_point_pos);
        glUniform2fv(glGetUniformLocation(shader.ID, "orbit_points"), 20 * 2, &orbit_points[0]);
        glUniform1i(glGetUniformLocation(shader.ID, "num_orbit_points"), num_orbit_points);

        // overwrite some uniform variables
        if (save_image)
        {
            glUniform1i(glGetUniformLocation(shader.ID, "draw_orbit_points"), 0);
        }

        // render the fractal
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // save image before the imgui widget is drawn to the screen buffer
        if (save_image)
        {
            GLubyte* pixels = new GLubyte[3 * width * heigth];
            glReadPixels(0, 0, width, heigth, GL_BGR, GL_UNSIGNED_BYTE, pixels);

            FIBITMAP* image = FreeImage_ConvertFromRawBits(pixels, width, heigth, 3 * width, 24, 0x0000FF, 0x00FF00, 0xFF0000, false);
            FreeImage_Save(FIF_PNG, image, image_save_path, 0);
            FreeImage_Unload(image);
            delete[] pixels;

            save_image = false;
        }

        // draw the imgui windows to the screen
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

// zoom in/out at the mouse position by scrolling
static void zoom_by_scrolling_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    if (!io.WantCaptureMouse) // check if mouse is not over imgui window
    {
        double xpos, ypos;
        get_cursor_pos(window, xpos, ypos);
        
        camera.zoom_at_point(xpos, ypos, yoffset);

        // current_cursor = ImGuiMouseCursor_ResizeNS; // TODO need a good way to reset cursor after scrolling
    }
}

static void drag_translation_callback(GLFWwindow* window, int button, int action, int mods)
{
    ImGuiIO& io = ImGui::GetIO(); (void)io; // for checking if the mouse is inside any imgui window

    // move around by holding the left mouse button and moving the mouse
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS && !io.WantCaptureMouse)
        {
            double xpos, ypos;
            get_cursor_pos(window, xpos, ypos);

            camera.drag_start(xpos, ypos);

            current_cursor = ImGuiMouseCursor_Hand;
        }
        else if (action == GLFW_RELEASE)
        {
            camera.drag_end();
            current_cursor = ImGuiMouseCursor_Arrow;
        }
    }
    // place the orbit points by clicking with the right mouse button
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && !io.WantCaptureMouse)
    {
        double xpos, ypos;
        get_cursor_pos(window, xpos, ypos);
        glm::vec2 pos_orbit_point = camera.convert_pixel_to_mandel(xpos, ypos);
        if (!change_orbit_point_pos == 0)
        {
            int orbit_point_pos_temp = 2 * (change_orbit_point_pos - 1);
            orbit_points[orbit_point_pos_temp] = pos_orbit_point.x;
            orbit_points[orbit_point_pos_temp + 1] = pos_orbit_point.y;
        }
    }
}

// get the current position of the mouse in pixels where (0, 0) is in the bottom left
void get_cursor_pos(GLFWwindow* window, double& xpos, double& ypos)
{
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);

    glfwGetCursorPos(window, &xpos, &ypos);
    ypos = (double)display_h - ypos; // set origin to bottom left
}

// setups the glfw window and the opengl context
GLFWwindow* setup()
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) { std::exit(EXIT_FAILURE); }

    // Decide GL+GLSL versions
    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(width, heigth, "ray marching", NULL, NULL);
    if (window == NULL) { std::exit(EXIT_FAILURE); }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
    bool err = gladLoadGL(glfwGetProcAddress) == 0;
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        std::exit(EXIT_FAILURE);
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = NULL; // don't make imgui.ini file after running application

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    return window;
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}
