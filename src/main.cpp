#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>

const std::string vertex_path = "src/shader.vert";
const std::string fragment_path = "src/shader.frag";

const int width = 1280;
const int heigth = 720;

glm::mat3 pixel_to_mandel;
const float scale_factor = 0.9;
bool drag_started = false;
double prev_cursor_x, prev_cursor_y;


static void zoom_by_scrolling_callback(GLFWwindow* window, double xoffset, double yoffset);
static void drag_translation_callback(GLFWwindow* window, int button, int action, int mods);
void get_cursor_pos(GLFWwindow* window, double& xpos, double& ypos);
static void glfw_error_callback(int error, const char* description);

int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

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
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
    bool err = gladLoadGL(glfwGetProcAddress) == 0;
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    glfwSetScrollCallback(window, zoom_by_scrolling_callback);
    glfwSetMouseButtonCallback(window, drag_translation_callback);

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

    // Shader shader = Shader(vertex_path, fragment_path);
    Shader shader = Shader(vertex_path, fragment_path);

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

    // for the mandelbrot starting range is; x: [-2.5, 1.0], y: [-1.0, 1.0]
    glm::mat3 scale_matrix = glm::scale(glm::mat3(1.0f), glm::vec2(3.5f / (float)width, 2.0f / (float)heigth));
    glm::mat3 translate_matrix = glm::translate(glm::mat3(1.0f), glm::vec2(-2.5f, -1.0f));
    pixel_to_mandel = translate_matrix * scale_matrix * glm::mat3(1.0f);

    // Our state
    bool show_demo_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        if (drag_started)
        {
            double xpos, ypos;
            get_cursor_pos(window, xpos, ypos);

            double xdiff = prev_cursor_x - xpos;
            double ydiff = prev_cursor_y - ypos;
            // only works because only scaling and translations are used
            xdiff *= pixel_to_mandel[0][0];
            ydiff *= pixel_to_mandel[1][1];

            prev_cursor_x = xpos;
            prev_cursor_y = ypos;

            glm::mat3 trans = glm::translate(glm::mat3(1.0f), glm::vec2(xdiff, ydiff));
            pixel_to_mandel = trans * pixel_to_mandel;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui windows
        {
            ImGui::Begin("Hello, world!");

            ImGui::Checkbox("Demo Window", &show_demo_window);
            ImGui::ColorEdit3("clear color", (float*)&clear_color);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            ImGui::End();
        }
        if (show_demo_window) { ImGui::ShowDemoWindow(&show_demo_window); }



        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();

        unsigned int uniform_buf = glGetUniformLocation(shader.ID, "pixel_to_mandel");
        glUniformMatrix3fv(uniform_buf, 1, GL_FALSE, glm::value_ptr(pixel_to_mandel));

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
    
    glm::vec3 mouse_pixel = glm::vec3(xpos, ypos, 1.0f);
    glm::vec3 mouse_mandel = pixel_to_mandel * mouse_pixel;
    mouse_mandel /= mouse_mandel.z;
    
    float temp_scale = (yoffset > 0) ? scale_factor : 1.0 / scale_factor;

    // move mouse to origin
    glm::mat3 trans = glm::translate(glm::mat3(1.0f), glm::vec2(-mouse_mandel.x, -mouse_mandel.y));
    // scale
    glm::mat3 scale = glm::scale(glm::mat3(1.0f), glm::vec2(temp_scale));
    // move back
    glm::mat3 trans_back = glm::translate(glm::mat3(1.0f), glm::vec2(mouse_mandel.x, mouse_mandel.y));
    // apply to pixel_to_mandel
    pixel_to_mandel = trans_back * scale * trans * pixel_to_mandel;
}

static void drag_translation_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            double xpos, ypos;
            get_cursor_pos(window, xpos, ypos);

            prev_cursor_x = xpos;
            prev_cursor_y = ypos;

            drag_started = true;
        }
        else if (action == GLFW_RELEASE)
        {
            drag_started = false;
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

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}
