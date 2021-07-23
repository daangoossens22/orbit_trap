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

// variables shared with shader
const int MAX_ORBIT_POINTS = 20;
const int MAX_PALETTE_COLORS = 20;

const std::string fractal_vertex_path = "src/fractal_shaders/shader.vert";
const std::string fractal_fragment_path = "src/fractal_shaders/shader.frag";

const int width = 1280;
const int heigth = 720;

Camera camera = Camera((float)width, (float)heigth);

static void zoom_by_scrolling_callback(GLFWwindow* window, double xoffset, double yoffset);
static void drag_translation_callback(GLFWwindow* window, int button, int action, int mods);
void get_cursor_pos(GLFWwindow* window, double& xpos, double& ypos);

GLFWwindow* setup();
static void glfw_error_callback(int error, const char* description);

bool change_orbit_point_pos[2 * MAX_ORBIT_POINTS] = { false } ;
int num_orbit_points = 1;
float orbit_points[2 * MAX_ORBIT_POINTS] = { 0.0f };


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

    bool draw_orbit_points = true;
    ImVec4 orbit_point_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

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

        // ImGui windows
        {
            ImGui::Begin("Hello, world!");

            ImGui::Checkbox("Demo Window", &show_demo_window);
            ImGui::SliderInt("num palette colors", &num_palette_colors, 1, 8);
            for (int i = 0; i < num_palette_colors; i++)
            {
                std::string label_name = "palette color " + std::to_string(i);
                ImGui::ColorEdit3(label_name.c_str(), (float*)&palette[i]);
            }

            ImGui::SliderFloat("coloring shift", &coloring_shift, 0.0f, 6.3f);

            ImGui::Checkbox("draw orbit points", &draw_orbit_points);
            ImGui::ColorEdit3("orbit_point_color", (float*)&orbit_point_color);
            ImGui::Checkbox("set orbit point 0 position (right mouse click)", &change_orbit_point_pos[0]);
            // ImGui::SliderFloat2("orbit point 0", &orbit_points[0], -1.0f, 1.0f);

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

        camera.move_camera_matrix_to_gpu(shader.ID);

        unsigned int sin_buf = glGetUniformLocation(shader.ID, "coloring_shift");
        glUniform1f(sin_buf, coloring_shift);

        glUniform1i(glGetUniformLocation(shader.ID, "num_palette_colors"), num_palette_colors);
        glUniform4fv(glGetUniformLocation(shader.ID, "palette"), num_palette_colors, (float*)&palette[0]);

        glUniform1i(glGetUniformLocation(shader.ID, "draw_orbit_points"), (int)draw_orbit_points);
        glUniform4fv(glGetUniformLocation(shader.ID, "orbit_point_color"), 1, (float*)&orbit_point_color);
        glUniform2fv(glGetUniformLocation(shader.ID, "orbit_points"), 20 * 2, &orbit_points[0]);
        glUniform1i(glGetUniformLocation(shader.ID, "num_orbit_points"), num_orbit_points);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

static void zoom_by_scrolling_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    double xpos, ypos;
    get_cursor_pos(window, xpos, ypos);
    
    camera.zoom_at_point(xpos, ypos, yoffset);
}

static void drag_translation_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            if (!io.WantCaptureMouse) // check if mouse is not over imgui window
            {
                double xpos, ypos;
                get_cursor_pos(window, xpos, ypos);

                camera.drag_start(xpos, ypos);

            }
        }
        else if (action == GLFW_RELEASE)
        {
            camera.drag_end();
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        get_cursor_pos(window, xpos, ypos);
        glm::vec2 pos_orbit_point = camera.convert_pixel_to_mandel(xpos, ypos);
        for (int i = 0; i < num_orbit_points; ++i)
        {
            if (change_orbit_point_pos[i])
            {
                orbit_points[2 * i] = pos_orbit_point.x;
                orbit_points[2 * i + 1] = pos_orbit_point.y;
            }
        }
    }
}

void get_cursor_pos(GLFWwindow* window, double& xpos, double& ypos)
{
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);

    glfwGetCursorPos(window, &xpos, &ypos);
    ypos = (double)display_h - ypos; // set origin to bottom left
}

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
